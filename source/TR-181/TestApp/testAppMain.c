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
    printf("Received event: %s\n", event->name);
    if(strcmp(event->name, "Device.X_RDK_WanManager.InitialScanComplete") == 0)
    {
        InitialScanComplete = true;
        printf("InitialScanComplete set to true\n");
    }
    else if(strcmp(event->name, "Device.X_RDK_WanManager.InterfaceWanUpStatus") == 0 )
    {
        // Get the event value (should be a string like "WANOE,1|ADSL,0|DSL,0")
        const char* dm_value = NULL;
        if(event->data)
        {
            rbusValue_t value = rbusObject_GetValue(event->data, "dm_value");
            if(value)
                dm_value = rbusValue_GetString(value, NULL);
        }
        if(dm_value)
        {
            printf("Event value: %s\n", dm_value);

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
                    printf("%s is %sREADY (status=%d)\n", iface, status == 1 ? "" : "NOT ", status);
                }
                else
                {
                    printf("Other interface %s status: %d\n", iface, status);
                }
                }
                token = strtok(NULL, "|");
            }
            free(valueCopy);
            }
            else
            {
            printf("Failed to allocate memory for valueCopy\n");
            }
        }
        else
        {
            printf("No dm_value found in event data\n");
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
        printf("Method %s invoked successfully\n", methodName);
        rbusObject_Release(outParams);
    }
    else
    {
        printf("Failed to invoke method %s: %d\n", methodName, err);
    }

    rbusObject_Release(inParams);
}



int main(int argc, char** argv)
{
    rbusHandle_t handle;
    rbusError_t err;

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

    // Main loop
    printf("Entering main loop. Press Ctrl+C to exit.\n");
    int waitTime = 0;
    while(!InitialScanComplete && waitTime < 60)
    {
        printf("Waiting for Initial scan complete ...\n");
        sleep(5);
        waitTime += 5;
    }
    if(!InitialScanComplete)
    {
        printf("Initial scan did not complete within 1 minute. Continuing.\n");
    }
    printf("%s is READY. Continuing...\n", CONTROLLED_IFACE);
    // Trigger rbus method call for "Device.X_RDK_WanManager.Interface.[WANOE].WanStart()"
    char methodName[128];
    snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].WanStart()", CONTROLLED_IFACE);
    triggerMethodCall(handle, methodName);

    // Wait for isControlledIfaceReady again if needed
    isControlledIfaceReady = false;
    while(!isControlledIfaceReady)
    {
        printf("Waiting for %s to be READY after WanStart...\n", CONTROLLED_IFACE);
        sleep(5);
    }
    printf("%s is READY after WanStart.\n", CONTROLLED_IFACE);

    // Trigger SelectionControlRequest method
    triggerMethodCall(handle, "Device.X_RDK_WanManager.SelectionControlRequest()");

    // Trigger Activate method for CONTROLLED_IFACE
    snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].Activate()", CONTROLLED_IFACE);
    triggerMethodCall(handle, methodName);

    // Wait for PRIMARY_IFACE to be READY
    while(!isPrimaryIfaceReady)
    {
        printf("Waiting for %s to be READY...\n", PRIMARY_IFACE);
        sleep(5);
    }
    printf("%s is READY.\n", PRIMARY_IFACE);
    // Wait for 2 minutes before deactivation
    printf("Waiting for 2 minutes restoration delay before deactivating %s...\n", CONTROLLED_IFACE);
    sleep(120);

    // Trigger Deactivate method for CONTROLLED_IFACE
    snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].Deactivate()", CONTROLLED_IFACE);
    triggerMethodCall(handle, methodName);

    // Trigger WanStop method for CONTROLLED_IFACE
    snprintf(methodName, sizeof(methodName), "Device.X_RDK_WanManager.Interface.[%s].WanStop()", CONTROLLED_IFACE);
    triggerMethodCall(handle, methodName);

    // Wait for isControlledIfaceReady to become false
    while(isControlledIfaceReady)
    {
        printf("Waiting for %s to NOT be READY after Deactivate/WanStop...\n", CONTROLLED_IFACE);
        sleep(5);
    }
    printf("%s is NOT READY after Deactivate/WanStop.\n", CONTROLLED_IFACE);

    // Trigger SelectionControlRelease method
    triggerMethodCall(handle, "Device.X_RDK_WanManager.SelectionControlRelease()");
    // Sleep for 30 seconds
    sleep(30);
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

    return 0;
}