
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_network.h"
#include <api_key.h>
#include <api_hal_gpio.h>
#include <api_inc_gpio.h>


#define MAIN_TASK_STACK_SIZE    (2048 * 2)
#define MAIN_TASK_PRIORITY      0
#define MAIN_TASK_NAME          "Main Test Task"

#define SECOND_TASK_STACK_SIZE    (2048 * 2)
#define SECOND_TASK_PRIORITY      1
#define SECOND_TASK_NAME          "Second Test Task"

static HANDLE mainTaskHandle = NULL;
static HANDLE secondTaskHandle = NULL;
static bool flag = false;

uint8_t FlightFlag = 0;
HANDLE sem1 = NULL;
bool FLIGHT=false;
void EventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_KEY_DOWN:
        //Trace(1,"key down, key:0x%02x",pEvent->param1);
        if(pEvent->param1 == KEY_POWER)
        {
            FlightFlag = !FlightFlag;
            
            Trace(1,"power key press down now. FlightFlag: %d", FlightFlag);
            
        }
        break;

        case API_EVENT_ID_NO_SIMCARD:
            Trace(1,"!!NO SIM CARD%d!!!!",pEvent->param1);
            break;

        case API_EVENT_ID_SYSTEM_READY:
            Trace(1,"system initialize complete");
            break;

        case API_EVENT_ID_NETWORK_REGISTERED_HOME:
        case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
        {
            uint8_t status;
            Trace(1,"network register success");
            bool ret = Network_GetAttachStatus(&status);
            if(!ret)
                Trace(1,"get attach staus fail");
            Trace(1,"attach status:%d",status);
            if(status == 0)
            {
                ret = Network_StartAttach();
                if(!ret)
                {
                    Trace(1,"network attach fail");
                }
            }
            else
            {
                Network_PDP_Context_t context = {
                    .apn        ="cmnet",
                    .userName   = ""    ,
                    .userPasswd = ""
                };
                Network_StartActive(context);
                Trace(1,"Network_StartActive start0000");
            }
            break;
        }
        case API_EVENT_ID_NETWORK_ATTACHED:
            Trace(1,"network attach success");
            uint8_t status;
            bool ret = Network_GetActiveStatus(&status);
            if(!ret)
                Trace(1,"get active staus fail");
            Trace(1,"active status:%d",status);
            Network_PDP_Context_t context = {
                .apn        ="cmnet",
                .userName   = ""    ,
                .userPasswd = ""
            };
            if(Network_StartActive(context)==true)
            {
                Trace(1,"Network_StartActive start1111 success");
            }
            else
            {
                Trace(1,"Network_StartActive start1111 fail");
            }
            ret = Network_GetActiveStatus(&status);
            if(!ret)
                Trace(1,"get active staus fail");
            Trace(1,"active status:%d",status);
            
            break;

        case API_EVENT_ID_NETWORK_ACTIVATED:
            Trace(1,"network activate success");
            uint8_t status1;
            bool ret1 = Network_GetActiveStatus(&status1);
            if(!ret1)
                Trace(1,"get active staus fail");
            Trace(1,"active status:%d",status1);
            sem1 = 1;
            flag = true;
            //if(FLIGHT ==true)
            {
                Trace(1,"network activate success FLIGHT ==true");
                
                FLIGHT=false;	

            }
            break;

        case API_EVENT_ID_NETWORK_CELL_INFO:
        {
            uint8_t number = pEvent->param1;
            Network_Location_t* location = pEvent->pParam1;
            Trace(2,"network cell infomation,serving cell number:1, neighbor cell number:%d",number-1);
            
            for(int i=0;i<number;++i)
            {
                Trace(2,"cell %d info:%d%d%d,%d%d%d,%d,%d,%d,%d,%d,%d",i,
				location[i].sMcc[0], location[i].sMcc[1], location[i].sMcc[2], 
				location[i].sMnc[0], location[i].sMnc[1], location[i].sMnc[2],
				location[i].sLac, location[i].sCellID, location[i].iBsic,
                location[i].iRxLev, location[i].iRxLevSub, location[i].nArfcn);
            }
            break;
        }
        default:
            break;
    }
}
    uint8_t x = 0;
    uint8_t y = 0; 
    int time  = 0;

void CreateSem(HANDLE* sem_)
{
    *sem_ = 0;
    // *sem = OS_CreateSemaphore(0);
}

void WaitSem(HANDLE* sem_)
{
    // OS_WaitForSemaphore(*sem,OS_WAIT_FOREVER);
    // OS_DeleteSemaphore(*sem);
    // *sem = NULL;
    while(*sem_ == 0)
    {
        //Trace(1,"FlightMode1 sem1=%d",sem1);
        OS_Sleep(1);
    }
        
    *sem_ = 0;
}

void network_attach_activate_handle()
{
	uint8_t status; 		
	bool ret = Network_GetAttachStatus(&status);
	if(!ret){
		Trace(1,"get attach staus failed");
		return;
	}
	Trace(1,"attach status:%d",status);
	if(0 == status){
		ret = Network_StartAttach();
		if(!ret){
			Trace(1,"network attach failed");
			return;
		}
	}else{
		ret = Network_GetActiveStatus(&status);
		if(!ret){
			Trace(1,"get active staus failed");
			return;
		}
		
		Trace(1,"active status:%d",status);
		if(0 == status){
			Network_PDP_Context_t context = {
				.apn		="cmnet",
				.userName	= "",
				.userPasswd = ""
			};
			Network_StartActive(context);
            
		}
	}
}

void SecondTask(void *pData)
{

    char ip[16];
    GPIO_config_t gpioInput = {
        .mode               = GPIO_MODE_INPUT,
        .pin                = GPIO_PIN4,
        .defaultLevel       = GPIO_LEVEL_HIGH,
    };
    GPIO_Init(gpioInput);
    OS_Sleep(1000);
    OS_Sleep(1000);
    OS_Sleep(1000);
    Trace(1,"Start!");


    while(1)
    {
        uint8_t status2; 
        //time++;
        
        bool ret2 = Network_GetActiveStatus(&status2);
		if(!ret2){
			Trace(1,"get active staus failed");
			return;
		}
		
		Trace(1," FlightMode active status:%d",status2);
        if((FlightFlag == 0)&&(x == 0)&&(FLIGHT==true) )
        { 
            y = 0;
            x = 1;
            
            Network_PDP_Context_t context = {
				.apn		="cmnet11",
				.userName	= "",
				.userPasswd = ""
			};
			Network_StartActive(context);
            Network_StartActive(context);
            Network_SetFlightMode(false);
            OS_Sleep(3000);
            

         
        }
        else if((FlightFlag == 1)&&(y == 0) )
        {
            y = 1;
            x = 0;

            Network_SetFlightMode(true);
	        flag=false;
            Trace(1,"FlightMode Open");
            FLIGHT=true;
        }

        if(0)
        {
            if(Network_GetIp(ip, sizeof(ip)))
            {
                Trace(1,"network local ip:%s",ip);
            }
            else
            {
                Trace(1,"network get local ip address fail");
            }
            for(int i=0;i<10;++i)
            {
                if(!Network_GetCellInfoRequst())
                {
                    Trace(1,"network get cell info fail");
                }
                OS_Sleep(5000);
            }

            flag = false;
        }
        OS_Sleep(1000);
    }
}

void MainTask(void *pData)
{
    API_Event_t* event=NULL;

    secondTaskHandle = OS_CreateTask(SecondTask,
        NULL, NULL, SECOND_TASK_STACK_SIZE, SECOND_TASK_PRIORITY, 0, 0, SECOND_TASK_NAME);

    while(1)
    {
        if(OS_WaitEvent(mainTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void flight_mode_Main(void)
{
    mainTaskHandle = OS_CreateTask(MainTask ,
        NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}


