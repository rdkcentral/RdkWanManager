#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rbus/rbus.h>

#define EVENT_COUNT 3

#define CONTROLLED_IFACE "WANOE" // Replace with actual interface name
#define PRIMARY_IFACE "DSL" // Replace with actual primary interface name
const char* eventList[EVENT_COUNT] = {
    "Device.X_RDK_WanManager.InterfaceActiveStatus",
    "Device.X_RDK_WanManager.InitialScanComplete",
    "Device.X_RDK_WanManager.InterfaceWanUpStatus"
};

bool InitialScanComplete = false;
bool isControlledIfaceReady = false;
bool isPrimaryIfaceReady = false;

void eventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    printf("[%s] Received event: %s\n", __func__, event->name);
    if(strcmp(event->name, "Device.X_RDK_WanManager.InitialScanComplete") == 0)
    {
        InitialScanComplete = true;
        printf("[%s] InitialScanComplete set to true\n", __func__);
    }
    else if(strcmp(event->name, "Device.X_RDK_WanManager.InterfaceWanUpStatus") == 0 )
    {
        // Get the event value (should be a string like "WANOE,1|ADSL,0|DSL,0")
        const char* dm_value = NULL;
        if(event->data)
        {
            rbusValue_t value = rbusObject_GetValue(event->data, "value");
            if(value)
                dm_value = rbusValue_GetString(value, NULL);
        }
        if(dm_value)
        {
            printf("[%s] Event value: %s\n", __func__, dm_value);

            char* valueCopy = strdup(dm_value);
            if(valueCopy)
            {
                char* token = strtok(valueCopy, "|");
                while(token)
                {
                    char iface[32];
                    int status = 0;
                    if(sscanf(token, "%31[^,],%d", iface, &status) == 2)
                    {
                        bool* ifaceReady = NULL;
                        if(strcmp(iface, CONTROLLED_IFACE) == 0)
                        {
                            ifaceReady = &isControlledIfaceReady;
                        }
                        else if(strcmp(iface, PRIMARY_IFACE) == 0)
                        {
                            ifaceReady = &isPrimaryIfaceReady;
                        }

                        if(ifaceReady)
                        {
                            *ifaceReady = (status == 1);
                            printf("[%s] %s is %sREADY (status=%d)\n", __func__, iface, status == 1 ? "" : "NOT ", status);
                        }
                        else
                        {
                            printf("[%s] Other interface %s status: %d\n", __func__, iface, status);
                        }
                    }
                    token = strtok(NULL, "|");
                }
                free(valueCopy);
            }
            else
            {
                printf("[%s] Failed to allocate memory for valueCopy\n", __func__);
            }
        }
        else
        {
            printf("[%s] No dm_value found in event data\n", __func__);
        }
    }
}

void triggerMethodCall(rbusHandle_t handle, const char* methodName)
{
    rbusObject_t inParams, outParams;
    rbusError_t err;

    rbusObject_Init(&inParams, NULL);

    err = rbusMethod_Invoke(handle, methodName, inParams, &outParams);
    if(err == RBUS_ERROR_SUCCESS)
    {
        printf("[%s] Method %s invoked successfully\n", __func__, methodName);
        rbusObject_Release(outParams);
    }
    else
    {
        printf("[%s] Failed to invoke method %s: %d\n", __func__, methodName, err);
    }

    rbusObject_Release(inParams);
}



int main(int argc, char** argv)
{
    rbusHandle_t handle;
    rbusError_t err;
    printf("\n=============================================\n");
    printf("   RDK WanManager Test Application Started   \n");
    printf("=============================================\n\n");
    // Initialize rbus
    err = rbus_open(&handle, "TestApp");
    if(err != RBUS_ERROR_SUCCESS)
    {
        printf("Failed to open rbus: %d\n", err);
        return 1;
    }

    // Subscribe to events
    for(int i = 0; i < EVENT_COUNT; ++i)
    {
        err = rbusEvent_Subscribe(handle, eventList[i], eventHandler, NULL, 0);
        if(err != RBUS_ERROR_SUCCESS)
        {
            printf("Failed to subscribe to event %s: %d\n", eventList[i], err);
        }
        else
        {
            printf("Subscribed to event: %s\n", eventList[i]);
        }
    }

    // State machine for WAN interface control
    typedef enum {
        STATE_WAIT_INITIAL_SCAN,
        STATE_WAN_START,
        STATE_WAIT_CONTROLLED_READY,
        STATE_SELECTION_CONTROL_REQUEST,
        STATE_ACTIVATE,
        STATE_WAIT_PRIMARY_READY,
        STATE_RESTORATION_DELAY,
        STATE_DEACTIVATE,
        STATE_WAN_STOP,
        STATE_WAIT_CONTROLLED_NOT_READY,
        STATE_SELECTION_CONTROL_RELEASE,
        STATE_DONE
    } AppState;

    AppState state = STATE_WAIT_INITIAL_SCAN;
    int waitTime = 0;
    char methodName[128];

    printf("\n********** Starting state machine. Press Ctrl+C to exit. ***************\n\n");

    while(state != STATE_DONE)
    {
        switch(state)
        {
            case STATE_WAIT_INITIAL_SCAN:
                printf("STATE_WAIT_INITIAL_SCAN : Waiting for Initial scan complete ...\n");
                while(!InitialScanComplete && waitTime < 60)
                {
                    sleep(5);
                    waitTime += 5;
                }
                if(!InitialScanComplete)
                {
                    printf("STATE_WAIT_INITIAL_SCAN :Initial scan did not complete within 1 minute. Continuing.\n");
                }
                printf("%s is READY. Continuing...\n", CONTROLLED_IFACE);
                printf("Transitioning from STATE_WAIT_INITIAL_SCAN to STATE_WAN_START\n\n");
                state = STATE_WAN_START;
                break;

            case STATE_WAN_START:
                snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].WanStart()", CONTROLLED_IFACE);
                triggerMethodCall(handle, methodName);
                isControlledIfaceReady = false;
                printf("Transitioning from STATE_WAN_START to STATE_WAIT_CONTROLLED_READY\n\n");
                state = STATE_WAIT_CONTROLLED_READY;
                break;

            case STATE_WAIT_CONTROLLED_READY:
                printf("STATE_WAIT_CONTROLLED_READY : Waiting for %s to be READY after WanStart...\n", CONTROLLED_IFACE);
                while(!isControlledIfaceReady)
                {
                    sleep(5);
                }
                printf("%s is READY after WanStart.\n", CONTROLLED_IFACE);
                printf("Transitioning from STATE_WAIT_CONTROLLED_READY to STATE_SELECTION_CONTROL_REQUEST\n\n");
                state = STATE_SELECTION_CONTROL_REQUEST;
                break;

            case STATE_SELECTION_CONTROL_REQUEST:
                triggerMethodCall(handle, "Device.X_RDK_WanManager.SelectionControlRequest()");
                sleep(20);
                printf("STATE_SELECTION_CONTROL_REQUEST : Selection control requested. Now activating %s...\n", CONTROLLED_IFACE);
                printf("Transitioning from STATE_SELECTION_CONTROL_REQUEST to STATE_ACTIVATE\n\n");
                state = STATE_ACTIVATE;
                break;

            case STATE_ACTIVATE:
                snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].Activate()", CONTROLLED_IFACE);
                triggerMethodCall(handle, methodName);
                printf("Transitioning from STATE_ACTIVATE to STATE_WAIT_PRIMARY_READY\n\n");
                state = STATE_WAIT_PRIMARY_READY;
                break;

            case STATE_WAIT_PRIMARY_READY:
                printf("STATE_WAIT_PRIMARY_READY : Waiting for %s to be READY...\n", PRIMARY_IFACE);
                while(!isPrimaryIfaceReady)
                {
                    sleep(5);
                }
                printf("STATE_WAIT_PRIMARY_READY : %s is READY.\n", PRIMARY_IFACE);
                printf("Transitioning from STATE_WAIT_PRIMARY_READY to STATE_RESTORATION_DELAY\n\n");
                state = STATE_RESTORATION_DELAY;
                break;

            case STATE_RESTORATION_DELAY:
                printf("STATE_RESTORATION_DELAY : Waiting for 2 minutes restoration delay before deactivating %s...\n", CONTROLLED_IFACE);
                sleep(120);
                printf("Transitioning from STATE_RESTORATION_DELAY to STATE_DEACTIVATE\n\n");
                state = STATE_DEACTIVATE;
                break;

            case STATE_DEACTIVATE:
                snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].Deactivate()", CONTROLLED_IFACE);
                triggerMethodCall(handle, methodName);
                printf("Transitioning from STATE_DEACTIVATE to STATE_WAN_STOP\n\n");
                state = STATE_WAN_STOP;
                break;

            case STATE_WAN_STOP:
                snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].WanStop()", CONTROLLED_IFACE);
                triggerMethodCall(handle, methodName);
                printf("Transitioning from STATE_WAN_STOP to STATE_WAIT_CONTROLLED_NOT_READY\n\n");
                state = STATE_WAIT_CONTROLLED_NOT_READY;
                break;

            case STATE_WAIT_CONTROLLED_NOT_READY:
                printf("STATE_WAIT_CONTROLLED_NOT_READY : Waiting for %s to NOT be READY after Deactivate/WanStop...\n", CONTROLLED_IFACE);
                while(isControlledIfaceReady)
                {
                    sleep(5);
                }
                printf("STATE_WAIT_CONTROLLED_NOT_READY : %s is NOT READY after Deactivate/WanStop.\n", CONTROLLED_IFACE);
                printf("Transitioning from STATE_WAIT_CONTROLLED_NOT_READY to STATE_SELECTION_CONTROL_RELEASE\n\n");
                state = STATE_SELECTION_CONTROL_RELEASE;
                break;

            case STATE_SELECTION_CONTROL_RELEASE:
                triggerMethodCall(handle, "Device.X_RDK_WanManager.SelectionControlRelease()");
                sleep(30);
                printf("Transitioning from STATE_SELECTION_CONTROL_RELEASE to STATE_DONE\n\n");
                state = STATE_DONE;
                break;

            default:
                state = STATE_DONE;
                break;
        }
    }
    // Unsubscribe from events
    for(int i = 0; i < EVENT_COUNT; ++i)
    {
        err = rbusEvent_Unsubscribe(handle, eventList[i]);
        if(err != RBUS_ERROR_SUCCESS)
        {
            printf("Failed to unsubscribe from event %s: %d\n", eventList[i], err);
        }
        else
        {            
            printf("Unsubscribed from event: %s\n", eventList[i]);
        }
    }
    rbus_close(handle);

    printf("\n\n\n=============================================\n");
    printf(" RDK WanManager Test Application Exiting .Bye Bye  \n");
    printf("=============================================\n\n");
    return 0;
}