/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


/* ---- Include Files ---------------------------------------- */

#include <errno.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>  /* for waitpid */
#include "wanmgr_utils.h"
#include "ipc_msg.h"

/* amount of time to sleep between collect process attempt if timeout was specified. */
#define COLLECT_WAIT_INTERVAL_MS 40
#define APP_TERMINATE_TIMEOUT (5 * MSECS_IN_SEC)

static void freeArgs(char **argv);
static int parseArgs(const char *cmd, const char *args, char ***argv);
static int strtol64(const char *str, char **endptr, int32_t base, int64_t *val);

static void freeArgs(char **argv)
{
   uint32_t i=0;
   while (argv[i] != NULL)
   {
     if ((argv[i]) != NULL)
     {
         free((argv[i]));
         (argv[i]) = NULL;
      }
      i++;
   }

   if (argv != NULL)
   {
      free(argv);
      argv = NULL;
   }
}

static int parseArgs(const char *cmd, const char *args, char ***argv)
{
    int numArgs=3, i, len, argIndex=0;
    bool inSpace=TRUE;
    const char *cmdStr;
    char **array;

    len = (args == NULL) ? 0 : strlen(args);

    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else
        {
            inSpace = FALSE;
        }
    }

    array = (char **) malloc ((numArgs) * sizeof(char *));
    if (array == NULL)
    {
        return -1;
    }

    /* locate the command name, last part of string */
    cmdStr = strrchr(cmd, '/');
    if (cmdStr == NULL)
    {
        cmdStr = cmd;
    }
    else
    {
        cmdStr++;
    }

    array[argIndex] = calloc (1, strlen(cmdStr) + 1);

    if (array[argIndex] == NULL)
    {
        CcspTraceError(("memory allocation of %d failed", strlen(cmdStr) + 1));
        freeArgs(array);
        return -1;
    }
    else
    {
        strcpy(array[argIndex], cmdStr);
        argIndex++;
    }

    inSpace = TRUE;
    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else if ((args[i] != ' ') && (inSpace))
        {
            int startIndex, endIndex;

            startIndex = i;
            endIndex = i;
            while ((endIndex < len) && (args[endIndex] != ' '))
            {
                endIndex++;
            }

            array[argIndex] = calloc(1,endIndex - startIndex + 1);
            if (array[argIndex] == NULL)
            {
                CcspTraceError(("memory allocation of %d failed", endIndex - startIndex + 1));
                freeArgs(array);
                return -1;
            }

            memcpy(array[argIndex], &args[startIndex], endIndex - startIndex );

            argIndex++;

            inSpace = FALSE;
        }
    }
    array[argIndex] = NULL;
    (*argv) = array;

    return 0;
}

static int strtol64(const char *str, char **endptr, int32_t base, int64_t *val)
{
   int ret=RETURN_OK;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoll(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = RETURN_ERROR;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}

int util_spawnProcess(const char *execName, const char *args, int * processId)
{
   int32_t pid;
   char **argv = NULL;
   int ret = RETURN_OK;

   if ((ret = parseArgs(execName, args, &argv)) != RETURN_OK)
   {
      CcspTraceError(("Failed to parse arguments %d\n",ret));
      return ret;
   }

   pid = fork();
   if (pid == 0)
   {
      int32_t devNullFd=-1, fd;

      /*
       * This is the child.
      */

      devNullFd = open("/dev/null", O_RDWR);
      if (devNullFd == -1)
      {
         CcspTraceError(("open of /dev/null failed"));
         exit(-1);
      }

      close(0);
      fd = devNullFd;
      dup2(fd, 0);

      close(1);
      fd = devNullFd;
      dup2(fd, 1);

      close(2);
      fd = devNullFd;
      dup2(fd, 2);

      if (devNullFd != -1)
      {
         close(devNullFd);
      }

      signal(SIGHUP, SIG_DFL);
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGILL, SIG_DFL);
      signal(SIGTRAP, SIG_DFL);
      signal(SIGABRT, SIG_DFL);  /* same as SIGIOT */
      signal(SIGFPE, SIG_DFL);
      signal(SIGBUS, SIG_DFL);
      signal(SIGSEGV, SIG_DFL);
      signal(SIGSYS, SIG_DFL);
      signal(SIGPIPE, SIG_DFL);
      signal(SIGALRM, SIG_DFL);
      signal(SIGTERM, SIG_DFL);
      signal(SIGUSR1, SIG_DFL);
      signal(SIGUSR2, SIG_DFL);
      signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
      signal(SIGPWR, SIG_DFL);
      signal(SIGWINCH, SIG_DFL);
      signal(SIGURG, SIG_DFL);
      signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
      signal(SIGSTOP, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGCONT, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      signal(SIGVTALRM, SIG_DFL);
      signal(SIGPROF, SIG_DFL);
      signal(SIGXCPU, SIG_DFL);
      signal(SIGXFSZ, SIG_DFL);

      execv(execName, argv);
      /* We should not reach this line.  If we do, exec has failed. */
      exit(-1);
   }

   freeArgs(argv);

   *processId = pid;

   return ret;
}

int util_signalProcess(int32_t pid, int32_t sig)
{
   int32_t rc;

   if (pid <= 0)
   {
      CcspTraceError(("bad pid %d", pid));
      return RETURN_ERROR;
   }

   if ((rc = kill(pid, sig)) < 0)
   {
      CcspTraceError(("invalid signal(%d) or pid(%d)", sig, pid));
      return RETURN_ERROR;
   }

   return RETURN_OK;
}

int util_getNameByPid(int pid, char *nameBuf, int nameBufLen)
{
   FILE *fp;
   char processName[BUFLEN_256];
   char filename[BUFLEN_256];
   int rval=-1;

   if (nameBuf == NULL || nameBufLen <= 0)
   {
      CcspTraceError(("invalid args, nameBuf=%p nameBufLen=%d", nameBuf, nameBufLen));
      return rval;
   }

   snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);
   if ((fp = fopen(filename, "r")) == NULL)
   {
      CcspTraceError(("could not open %s", filename));
   }
   else
   {
      int rc, p;
      memset(nameBuf, 0, nameBufLen);
      memset(processName, 0, sizeof(processName));
      rc = fscanf(fp, "%d (%255s", &p, processName);
      fclose(fp);

      if (rc >= 2)
      {
         int c=0;
         while (c < nameBufLen-1 &&
                processName[c] != 0 && processName[c] != ')')
         {
            nameBuf[c] = processName[c];
            c++;
         }
         rval = 0;
      }
      else
      {
         CcspTraceError(("fscanf of %s failed", filename));
      }
   }

   return rval;
}

int util_getPidByName(const char *name)
{
   DIR *dir;
   FILE *fp;
   struct dirent *dent;
   BOOL found=FALSE;
   int pid, rc, p, i;
   int rval = 0;
   char processName[BUFLEN_256];
   char filename[BUFLEN_256];

   if (NULL == (dir = opendir("/proc")))
   {
      CcspTraceError(("could not open /proc"));
      return rval;
   }

   while (!found && (dent = readdir(dir)) != NULL)
   {
      if ((dent->d_type == DT_DIR) &&
          (RETURN_OK == strtol64(dent->d_name, NULL, 10, (int64_t*)&pid)))
      {
         snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);
         if ((fp = fopen(filename, "r")) == NULL)
         {
            CcspTraceError(("could not open %s", filename));
         }
         else
         {
            memset(processName, 0, sizeof(processName));
            rc = fscanf(fp, "%d (%255s", &p, processName);
            fclose(fp);

            if (rc >= 2)
            {
               i = strlen(processName);
               if (i > 0)
               {
                  if (processName[i-1] == ')')
                     processName[i-1] = 0;
               }
            }

            if (!strcmp(processName, name))
            {
               rval = pid;
               found = TRUE;
            }
         }
      }
   }

   closedir(dir);

   return rval;
}

int util_collectProcess(int pid, int timeout)
{
   int32_t rc, status, waitOption=0;
   int32_t requestedPid=-1;
   uint32_t timeoutRemaining=0;
   uint32_t sleepTime;
   int ret=RETURN_ERROR;

   requestedPid = pid;
   timeoutRemaining = timeout;

   if(timeoutRemaining > 0)
     waitOption = WNOHANG;

   timeoutRemaining = (timeoutRemaining <= 1) ?
                      (timeoutRemaining + 1) : timeoutRemaining;
   while (timeoutRemaining > 0)
   {
      rc = waitpid(requestedPid, &status, waitOption);
      if (rc == 0)
      {
         if (timeoutRemaining > 1)
         {
            sleepTime = (timeoutRemaining > COLLECT_WAIT_INTERVAL_MS) ?
                         COLLECT_WAIT_INTERVAL_MS : timeoutRemaining - 1;
            usleep(sleepTime * USECS_IN_MSEC);
            timeoutRemaining -= sleepTime;
         }
         else
         {
            timeoutRemaining = 0;
         }
      }
      else if (rc > 0)
      {
         pid = rc;
         timeoutRemaining = 0;
         ret = RETURN_OK;
      }
      else
      {
         if (errno == ECHILD)
         {
            CcspTraceInfo(("Could not collect child pid %d, possibly stolen by SIGCHLD handler?", requestedPid));
            ret = RETURN_ERROR;
         }
         else
         {
            CcspTraceError(("bad pid %d, errno=%d", requestedPid, errno));
            ret = RETURN_ERROR;
         }

         timeoutRemaining = 0;
      }
   }

   return ret;
}

int util_runCommandInShell(char *command)
{
   int pid;

   CcspTraceInfo(("executing %s\n", command));
   pid = fork();
   if (pid == -1)
   {
      CcspTraceError(("fork failed!"));
      return -1;
   }

   if (pid == 0)
   {
      /* this is the child */
      int i;
      char *argv[4];

      /* close all of the child's other fd's */
      for (i=3; i <= 50; i++)
      {
         close(i);
      }

      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      execv("/bin/sh", argv);
      CcspTraceError(("Should not have reached here!"));
      exit(127);
   }

   /* parent returns the pid */
   return pid;
}

int util_runCommandInShellBlocking(char *command)
{
   int processId;
   int timeout = 0;
   if ( command == 0 )
      return 1;

   if((processId = util_runCommandInShell(command) ) < 0)
   {
      return 1;
   }
   if(util_collectProcess(processId, timeout) != 0)
   {
      CcspTraceError(("Process collection failed\n"));
      return -1;
   }
   return 0;
}

bool util_isFilePresent(char *filename)
{
   struct stat statbuf;
   int32_t rc;

   rc = stat(filename, &statbuf);

   if (rc == 0)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


static int GetRunTimeRootDir(char *pathBuf, uint32_t pathBufLen)
{

    uint32_t rc;

    rc = snprintf(pathBuf, pathBufLen, "/");
    if (rc >= pathBufLen)
    {
      AnscTraceError((" %s - %d : pathBufLen %d is too small for buf %s \n", __FUNCTION__,__LINE__, pathBufLen, pathBuf));
      return RETURN_ERROR;
    }

    return RETURN_OK;
}

int GetPathToApp(const char *name, char *buf, uint32_t len)
{
    char envPathBuf[BUFLEN_1024] = {0};
    char rootDirBuf[MAX_FULLPATH_LENGTH] = {0};
    uint32_t i = 0;
    uint32_t envPathLen;
    int32_t rc;
    char *start;
    int ret;

    ret = GetRunTimeRootDir(rootDirBuf, sizeof(rootDirBuf));
    if (ret != RETURN_OK)
    {
        CcspTraceError(("%s  %d : GetRunTimeRootDir returns error ret=%d \n", __FUNCTION__,__LINE__, ret));
        return ret;
    }

    rc = snprintf(envPathBuf, sizeof(envPathBuf), "%s", getenv("PATH"));
    if (rc >= (int32_t)sizeof(envPathBuf))
    {
        CcspTraceError(("envPathBuf (size=%d) is too small to hold PATH env %s \n",
                        sizeof(envPathBuf), getenv("PATH")));
        return RETURN_ERROR;
    }

    envPathLen = strlen(envPathBuf);
    start = envPathBuf;

    while (i <= envPathLen)
    {
        if (envPathBuf[i] == ':' || envPathBuf[i] == '\0')
        {
            envPathBuf[i] = 0;
            rc = snprintf(buf, len, "%s%s/%s", rootDirBuf, start, name);
            if (rc >= (int32_t)len)
            {
                CcspTraceError(("MAX_FULLPATH_LENGTH (%d) exceeded on path %s \n",
                                MAX_FULLPATH_LENGTH, buf));
                return RETURN_ERROR;
            }

            if (util_isFilePresent(buf))
            {
                return RETURN_OK;
            }
            i++;
            start = &(envPathBuf[i]);
        }
        else
        {
            i++;
        }
    }

    CcspTraceInfo(("not found==>%s \n", name));
    return RETURN_ERROR;
}

int WanManager_DoSystemActionWithStatus(const char *from, char *cmd)
{
    CcspTraceInfo(("%s -- %s \n", from, cmd));
    return util_runCommandInShellBlocking(cmd);
}

void WanManager_DoSystemAction(const char *from, char *cmd)
{
    if (RETURN_OK != WanManager_DoSystemActionWithStatus(from, cmd))
    {
        CcspTraceInfo(("WanManager_DoSystemActionWithStatus Failed!!!\n"));
    }
}


BOOL WanManager_IsApplicationRunning(const char *appName)
{
    BOOL isAppRuning = TRUE;

    if (util_getPidByName(appName) <= 0)
        isAppRuning = FALSE;

    return isAppRuning;
}

static int LaunchApp(const char *appName, const char *cmdLineArgs)
{
    char exeBuf[MAX_FULLPATH_LENGTH] = {0};
    int ret = RETURN_OK;
    int pid;

    if (ANSC_STATUS_SUCCESS != GetPathToApp(appName, exeBuf, sizeof(exeBuf) - 1))
    {
        CcspTraceError(("Could not find requested app %s, not launched! \n",
                        appName));
        return -1;
    }

    CcspTraceInfo(("spawning %s args %s\n", appName, cmdLineArgs));
    ret = util_spawnProcess(&exeBuf, cmdLineArgs, &pid);
    if (ret != RETURN_OK)
    {
        CcspTraceError(("could not spawn child %s args %s \n", exeBuf, cmdLineArgs));
    }
    else
    {
        CcspTraceError(("%s launched, pid %d\n", appName, pid));
    }
    return pid;
}

void CollectApp(int pid)
{
    int ret;
    int timeout = APP_TERMINATE_TIMEOUT;

    if ((ret = util_collectProcess(pid, timeout)) != RETURN_OK)
    {
        CcspTraceError(("Could not collect pid=%d timeout=%dms, ret=%d.  Kill it with SIGKILL. \n", pid, timeout, ret));
        /*
         * Send SIGKILL and collect the process, otherwise,
         * we end up with a zombie process.
         */
        util_signalProcess(pid, SIGKILL);
        if ((ret = util_collectProcess(pid, timeout)) != RETURN_OK)
        {
            CcspTraceError(("Still could not collect pid=%d after SIGKILL, ret=%d \n", pid, ret));
            /* this process is really stuck.  Not much I can do now.
             * Just leave it running I guess. */
        }
        else
        {
            CcspTraceInfo(("collected pid %d after SIGKILL \n", pid));
        }
    }
    else
    {
        CcspTraceInfo(("collected pid %d\n", pid));
    }
    return;
}

int WanManager_DoStartApp(const char *appName, const char *cmdLineArgs)
{
    int pid = 0;
    char command[BUFLEN_1024];
    if (NULL == appName)
    {
        CcspTraceError(("Application name is not given \n"));
        return 0;
    }

    CcspTraceInfo(("Starting %s - with parameters %s \n", appName, cmdLineArgs));

    pid = LaunchApp(appName, cmdLineArgs);
    /* Start application. */
    if (pid < 0)
    {
        CcspTraceError(("Failed to start %s application \n", appName));
        return 0;
    }

    /* Get PID and return it. */
    return pid;
}

void WanManager_DoStopApp(const char *appName)
{
    int pid;

    pid = util_getPidByName(appName);
    if (pid > 0)
    {
        CcspTraceInfo(("Stopping  %s \n", appName));
        util_signalProcess(pid, SIGTERM);
        CollectApp(pid);
    }
}

int WanManager_DoRestartApp(const char *appName, const char *cmdLineArgs)
{
    if (NULL == appName)
    {
        CcspTraceError(("Application name is not given \n"));
        return 0;
    }

    CcspTraceInfo(("Restarting %s - with parameters %s \n", appName, cmdLineArgs));

    /* Stop App if it is running. */
    if (WanManager_IsApplicationRunning(appName) == TRUE)
    {
        WanManager_DoStopApp(appName);
        CcspTraceInfo(("App %s stopped \n", appName));
    }
    /* Restart Application. */
    return WanManager_DoStartApp(appName, cmdLineArgs);
}


void WanManager_DoCollectApp(const char *appName)
{
    int pid = 0;

    pid = util_getZombiePidByName(appName);
    if (pid > 0)
    {
        CcspTraceInfo(("%s [%d]: Stopping %s zombie pid:%d\n", __FUNCTION__, __LINE__, appName, pid));
        CollectApp(pid);
    }
}


#ifdef FEATURE_IPOE_HEALTH_CHECK
UINT WanManager_StartIpoeHealthCheckService(const char *ifName)
{
    if (ifName == NULL)
    {
        CcspTraceError(("could not start %s. Invalid intfername name.\n", IHC_CLIENT_NAME));
        return 0;
    }

    int IhcPid = 0;
    char cmdArgs[BUFLEN_128] = {0};

    snprintf(cmdArgs, BUFLEN_128, "-i %s", ifName);
    CcspTraceInfo(("%s [%d]: Starting IHC with args - %s %s\n", __FUNCTION__, __LINE__, IHC_CLIENT_NAME, cmdArgs));

    IhcPid = WanManager_DoStartApp(IHC_CLIENT_NAME, cmdArgs);
    if (IhcPid == 0)
    {
        CcspTraceError(("%s [%d]: could not start %s", __FUNCTION__, __LINE__, IHC_CLIENT_NAME));
        return 0;
    }
    return IhcPid;
}

ANSC_STATUS WanManager_StopIpoeHealthCheckService(UINT IhcPid)
{
    if (IhcPid == 0 )
    {
        CcspTraceError(("Invalid PID %d. Cannot Stop IHC App.\n", IhcPid));
        return ANSC_STATUS_FAILURE;
    }
    util_signalProcess(IhcPid, SIGTERM);
    CollectApp(IhcPid);
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS readIAPDPrefixFromFile(char *prefix, int buflen, int *plen, int *pltime, int *vltime)
{
    FILE *fp = NULL;
    char *conffile = DHCP6C_RENEW_PREFIX_FILE;
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    char prefixAddr[128];
    int prefixLen = 0;
    int prefLifeTime = 0;
    int validLifeTime = 0;

    if ((fp = fopen(conffile,"r")) == NULL)
    {
        CcspTraceError(("Unable to open renew prefix file \n"));
        ret = ANSC_STATUS_FAILURE;
    }
    else
    {
        /* Read the IA_PD information in the variable */
        (void)fscanf(fp, "%127s %d %d %d",prefixAddr, &prefixLen, &prefLifeTime, &validLifeTime);
        CcspTraceInfo(("prefixAddr:%s buflen:%d prefixLen:%d prefLifeTime:%d validLifeTime:%d",
            prefixAddr, buflen, prefixLen, prefLifeTime, validLifeTime));
        /* Copy the IA_PD prefix and delete the prefix file */
        if((prefix != NULL) && (strlen(prefixAddr) <= buflen))
        {
            strncpy(prefix, prefixAddr, strlen(prefixAddr));
            *plen = prefixLen;
            *pltime = prefLifeTime;
            *vltime = validLifeTime;
        }
        else
        {
            ret = ANSC_STATUS_FAILURE;
        }
        fclose(fp);
        unlink(conffile);
    }

    return ret;
}
#endif



/* * WanManager_GetDateAndUptime() */
void WanManager_GetDateAndUptime( char *buffer, int *uptime )
{
    struct  timeval  tv;
    struct  tm       *tm;
    struct  sysinfo info;
    char    fmt[ 64 ], buf [64] = {0};

    sysinfo( &info );
    gettimeofday( &tv, NULL );

    if( (tm = localtime( &tv.tv_sec ) ) != NULL)
    {
        strftime( fmt, sizeof( fmt ), "%y%m%d-%T.%%06u", tm );
        snprintf( buf, sizeof( buf ), fmt, tv.tv_usec );
    }

    snprintf( buffer, sizeof(buf)-1, "%s", buf);

    *uptime= info.uptime;
}

uint32_t WanManager_getUpTime()
{
    struct sysinfo info;
    sysinfo( &info );
    return( info.uptime );
}

/*
 * util_getZombiePidByName  ()
 * @description: check the contents of /proc directory to match the process name
 * @params     : name - process name
 * @return     : returns the Zombie pid if proc entry exists
 */
int util_getZombiePidByName (char * name)
{
    if (name == NULL)
    {
        CcspTraceInfo(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return 0;
    }

    DIR *dir;
    FILE *fp;
    struct dirent *dent;
    bool found=false;
    int pid, rc, p, i;
    int rval = 0;
    char processName[BUFLEN_256];
    char filename[BUFLEN_256];
    char status = 0;

    if (NULL == (dir = opendir("/proc")))
    {
        CcspTraceInfo(("%s %d:could not open /proc\n", __FUNCTION__, __LINE__));
        return 0;
    }

    while (!found && (dent = readdir(dir)) != NULL)
    {
        if ((dent->d_type == DT_DIR) &&
                (RETURN_OK == strtol64(dent->d_name, NULL, 10, (int64_t*)&pid)))
        {
            snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);
            if ((fp = fopen(filename, "r")) == NULL)
            {
                CcspTraceInfo(("%s %d:could not open %s\n", __FUNCTION__, __LINE__, filename));
            }
            else
            {
                memset(processName, 0, sizeof(processName));
                status = 0;
                rc = fscanf(fp, "%d (%255s %c ", &p, processName, &status);
                fclose(fp);

                if (rc >= 2)
                {
                    i = strlen(processName);
                    if (i > 0)
                    {
                        if (processName[i-1] == ')')
                            processName[i-1] = 0;
                    }
                }

                if (!strcmp(processName, name))
                {
	            if (status == 'Z')
                    {
                        rval = pid;
                        found = true;
                        CcspTraceInfo(("%s %d: processName=[%s], %s running in %c mode\n", __FUNCTION__, __LINE__, processName, filename, status));
                    }
                }
            }
        }
    }

    closedir(dir);

    return rval;

}
