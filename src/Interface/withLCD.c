#include "withLCD.h"
#include <stddef.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "YX_Define.h"
#include "YC_Define.h"
#include "interface.h"
#include "sys.h"
LCD_YC_YX_DATA yc_data[MAX_TOTAL_PCS_NUM];
LCD_YC_YX_DATA zjyc_data[MAX_LCD_NUM];
// short Yc_PW_Data[MAX_TOTAL_PCS_NUM];//
unsigned int Yx_Pcs_Status = 0;
unsigned char flag_RecvNeed_PCS[]={0,0,0,0,0,0};
PARA_61850 Frome61850;
PARA_61850 *pFrome61850 = (PARA_61850 *)&Frome61850;

YKOrder ykOrder_61850 = NULL;
unsigned char yc_Send_Flag[][4] = {
	{Line_AB_voltage, 1, 0, 1},
	{Line_BC_voltage, 1, 0, 1},
	{Line_CA_voltage, 1, 0, 1},
	{Phase_A_current, 1, 1, 0},
	{Phase_B_current, 1, 1, 0},
	{Phase_C_current, 1, 1, 0},
	{Power_factor, 1, 0, 1},
	{Frequency, 1, 0, 1},
	{Active_power, 1, 0, 0},
	{Reactive_power, 1, 0, 1},
	{Apparent_power, 1, 0, 1},
};

YX_SEND_FLAG yx_send_flag[] = {
	{0, bPcsStoped, 1, 0, 0},
	{0, bSavingStatus, 1, 0, 0},
	{0, bFaultStatus, 1, 0, 0},
	{0, bFffLineRunning, 1, 0, 0},
	{0, bMergeCircuit, 1, 0, 0},
	{0, bPcsRunning, 1, 0, 0},

	{1, bConnectMode, 1, 0, 0},

	{4, bConstPwDischargeMode, 1, 0, 0},
	{4, bConstPwChargeMode, 1, 0, 0},
	{4, bConstCurCharging, 1, 0, 0},
	{4, bConstCurDischarging, 1, 0, 0},

};

unsigned char pcs_fault_flag[MAX_TOTAL_PCS_NUM];

SendTo61850 yc_realtime_tab[] = {

	{Line_AB_voltage, 1, _FLOAT_, 4, 2, 10},
	{Line_BC_voltage, 2, _FLOAT_, 4, 2, 10},
	{Line_CA_voltage, 3, _FLOAT_, 4, 2, 10},
	{Phase_A_current, 4, _FLOAT_, 4, 2, 10},
	{Phase_B_current, 5, _FLOAT_, 4, 2, 10},
	{Phase_C_current, 6, _FLOAT_, 4, 2, 10},
	{Power_factor, 7, _FLOAT_, 4, 2, 1000},
	{Frequency, 8, _FLOAT_, 4, 2, 100},		 //电网频率
	{Active_power, 9, _FLOAT_, 4, 2, 10},	 //交流有功功率
	{Reactive_power, 10, _FLOAT_, 4, 2, 10}, //交流无功功率
	{Apparent_power, 11, _FLOAT_, 4, 2, 10}	 //交流视在功率
};

SendTo61850 zjyc_realtime_tab[] = {

	{Line_AB_voltage_zj, 8, _FLOAT_, 4, 2, 10},	 // 0x1177	"电网AB线电压   整机	int16	0.1 V	R
	{Line_BC_voltage_zj, 9, _FLOAT_, 4, 2, 10},	 // 0x1178	"电网BC线电压   整机	int16	0.1 V	R
	{Line_CA_voltage_zj, 10, _FLOAT_, 4, 2, 10}, // 0x1179	"电网CA线电压   整机	int16	0.1 V	R
	{Phase_A_current_zj, 11, _FLOAT_, 4, 2, 10}, // 0x117A	"电网A相电流    整机	int16	0.1 A	R
	{Phase_B_current_zj, 12, _FLOAT_, 4, 2, 10}, // 0x117B	"电网B相电流    整机	int16	0.1 A	R
	{Phase_C_current_zj, 13, _FLOAT_, 4, 2, 10}, // 0x117C	"电网C相电流    整机	int16	0.1 A	R
	{Frequency_zj, 14, _FLOAT_, 4, 2, 100},		 // 0x117E	"电网频率       整机	int16	0.01 Hz	R
	{Power_factor_zj, 15, _FLOAT_, 4, 2, 1000},	 // 0x117D	"功率因数       整机	int16	0.001	R
	{Active_power_zj, 16, _FLOAT_, 4, 2, 10},	 // 0x117F	"交流有功功率   整机	int16	0.1kW	R
	{Reactive_power_zj, 17, _FLOAT_, 4, 2, 10},	 // 0x1180	"交流无功功率   整机	int16	0.1kVar
	{Apparent_power_zj, 18, _FLOAT_, 4, 2, 10},	 // 0x1181	"交流视在功率   整机	int16	0.1kVA	R
	{DC_power_input_zj, 19, _FLOAT_, 4, 2, 10}	 // 0x1076    "直流功率"	整机	int16	0.1 kW	R
};
SendTo61850_count yc_count_tab[] = {

	{Line_AB_voltage, 8, _FLOAT_, 4, 2, 10, 2},
	{Line_BC_voltage, 9, _FLOAT_, 4, 2, 10, 2},
	{Line_CA_voltage, 10, _FLOAT_, 4, 2, 10, 2},
	{Phase_A_current, 11, _FLOAT_, 4, 2, 10, 1},
	{Phase_B_current, 12, _FLOAT_, 4, 2, 10, 1},
	{Phase_C_current, 13, _FLOAT_, 4, 2, 10, 1},
	{Power_factor, 14, _FLOAT_, 4, 2, 1000, 0}, //功率因数
	{Frequency, 15, _FLOAT_, 4, 2, 100, 2},		//电网频率
	{Active_power, 16, _FLOAT_, 4, 2, 10, 2},	//交流有功功率
	{Reactive_power, 17, _FLOAT_, 4, 2, 10, 2}, //交流无功功率
	{Apparent_power, 18, _FLOAT_, 4, 2, 10, 2}, //交流视在功率
	{DC_power_input, 19, _FLOAT_, 4, 2, 10, 1}	//

};

// SendTo61850 yc_realtime_tab[] = {

// 	{Line_AB_voltage, 1, _FLOAT_, 4, 2, 10},
// 	{Line_BC_voltage, 2, _FLOAT_, 4, 2, 10},
// 	{Line_CA_voltage, 3, _FLOAT_, 4, 2, 10},
// 	{Phase_A_current, 4, _FLOAT_, 4, 2, 10},
// 	{Phase_B_current, 5, _FLOAT_, 4, 2, 10},
// 	{Phase_C_current, 6, _FLOAT_, 4, 2, 10},
// 	{Power_factor, 7, _FLOAT_, 4, 2, 1000},
// 	{Frequency, 8, _FLOAT_, 4, 2, 100},		 //电网频率
// 	{Active_power, 9, _FLOAT_, 4, 2, 10},	 //交流有功功率
// 	{Reactive_power, 10, _FLOAT_, 4, 2, 10}, //交流无功功率
// 	{Apparent_power, 11, _FLOAT_, 4, 2, 10}	 //交流视在功率
// };
int LcdTo61850_YC(unsigned char lcdid, unsigned char pcsid, unsigned short *pdata)
{
	int i = 0;
	MyData senddata;
	int ret = 0xff;
	unsigned char b1, b2;
	float temp;
	short temp_i;

	for (i = 0; i < 11; i++)
	{
		senddata.data_info[i].sAddr.portID = INFO_PCS;
		senddata.data_info[i].sAddr.devID = lcdid * 6 + pcsid;
		senddata.data_info[i].sAddr.pointID = yc_realtime_tab[i].pointID;
		senddata.data_info[i].sAddr.typeID = yc_realtime_tab[i].typeID;
		senddata.data_info[i].data_size = yc_realtime_tab[i].data_size;
		senddata.data_info[i].el_tag = yc_realtime_tab[i].el_tag;

		if (yc_realtime_tab[i].el_tag == _FLOAT_)
		{
			b1 = pdata[yc_realtime_tab[i].pos_protocol] / 256;
			b2 = pdata[yc_realtime_tab[i].pos_protocol] % 256;
			temp_i = b2 * 256 + b1;
			temp = (float)temp_i / yc_realtime_tab[i].precision;
			*(float *)&senddata.data_info[i].data = temp;
		}
	}
	senddata.num = 11;
	ret = sendtotask(&senddata);
	return ret;
}

static int countSumAve_zjyc_Send(void)
{
	int data_Active_power;	 //交流有功功率
	int data_Reactive_power; //交流无功功率
	int data_Apparent_power; //交流视在功率
	int i;
	int n = ARRAY_LEN(zjyc_realtime_tab);
	int temp;
	int ret;
	MyData senddata;

	for (i = 0; i < pFrome61850->lcdnum; i++)
	{

		data_Active_power += (int)zjyc_data[i].pcs_data[Active_power_zj];
		// temp[9]=data_Active_power/10;

		data_Reactive_power += (int)zjyc_data[i].pcs_data[Reactive_power_zj];
		// temp[10]=data_Reactive_power/10;
		data_Apparent_power += (int)zjyc_data[i].pcs_data[Apparent_power_zj];
		// temp[11]=data_Apparent_power/10;
	}

	for (i = 0; i < n; i++)
	{
		if (zjyc_realtime_tab[i].pos_protocol == Active_power_zj)
			temp = data_Active_power;
		else if (zjyc_realtime_tab[i].pos_protocol == Reactive_power_zj)
			temp = data_Reactive_power;
		else if (zjyc_realtime_tab[i].pos_protocol == Apparent_power_zj)
			temp = data_Apparent_power;
		else
		{
			temp = (int)zjyc_data[i].pcs_data[zjyc_realtime_tab[i].pos_protocol];
		}

		senddata.data_info[i].sAddr.portID = 1;
		senddata.data_info[i].sAddr.devID = 1;
		senddata.data_info[i].sAddr.typeID = 2;
		senddata.data_info[i].data_size = 4;
		senddata.data_info[i].el_tag = _FLOAT_;
		senddata.data_info[i].sAddr.pointID = zjyc_realtime_tab[i].pointID;

		*(float *)&senddata.data_info[senddata.num].data[0] = ((float)temp) / zjyc_realtime_tab[i].precision;
	}

	senddata.num = n;
	ret = sendtotask(&senddata);

	if (ret == 1)
	{
		printf("1充放电数据上传成功！！！\n");
	}
	else
		printf("2充放电数据上传成功失败！！！\n");

	return 0;
}
static int countSumAve_yc_Send(void)
{
	int sumdata[20];
	int i, j;
	// int margin;
	MyData senddata;
	int n = ARRAY_LEN(yc_count_tab);
	int m = 0; //统计故障pcs
	float temp;
	unsigned char b1, b2;
	int ret;
	for (i = 0; i < 20; i++)
	{
		sumdata[i] = 0;
	}
	for (i = 0; i < n; i++)
	{
		if (yc_count_tab[i].flag == 0)
		{

			continue;
		}
		for (j = 0; j < total_pcsnum; j++)
		{

			if (pcs_fault_flag[j] == 0)
			{
				b1 = yc_data[j].pcs_data[yc_count_tab[i].pos_protocol] % 256;
				b2 = yc_data[j].pcs_data[yc_count_tab[i].pos_protocol] / 256;
				sumdata[i] += (int)(b1 * 256 + b2);
			}
			else
				m++;
		}
	}

	for (i = 0; i < n; i++)
	{

		if (yc_count_tab[i].flag == 2) //求平均
		{
			sumdata[i] /= (total_pcsnum - m); // yc_count_tab[i].precision;
		}
	}
	for (i = 0; i < n; i++)
	{
		senddata.data_info[i].sAddr.portID = 1;
		senddata.data_info[i].sAddr.devID = 1;
		senddata.data_info[i].sAddr.typeID = 2;
		senddata.data_info[i].data_size = yc_count_tab[i].data_size;
		senddata.data_info[i].el_tag = yc_count_tab[i].el_tag;
		senddata.data_info[i].sAddr.pointID = yc_count_tab[i].pointID;
		if (yc_count_tab[i].el_tag == _FLOAT_)
		{
			temp = (float)sumdata[i] / yc_realtime_tab[i].precision;
			*(float *)&senddata.data_info[i].data[0] = temp;
		}
	}
	senddata.num = n;
	// margin = Ave_Max_PW * (total_pcsnum - m) - sumdata[10];
	ret = sendtotask(&senddata);

	if (ret == 1)
	{
		printf("遥测统计数据上传成功！！！\n");
	}
	else
		printf("遥测统计数据上传成功失败！！！\n");
	return 0;
}

static int countSumAve_yc_Send1(void)
{
	float sum_dpw = 0;
	float sum_pw = 0;
	short temp;
	int i;
	MyData senddata;
	int ret;
	for (i = 0; i < total_pcsnum; i++)
	{
		if (pcs_fault_flag[i] != 0)
			continue;
		temp = (short)yc_data[i].pcs_data[DC_power_input];
		if (temp > 0)
		{
			sum_pw += ((float)temp / 10);
		}
		else
		{
			temp = -temp;
			sum_dpw += ((float)temp / 10);
		}
	}
	senddata.num = 0;

	//	if(sum_pw>0)
	{

		senddata.data_info[senddata.num].sAddr.portID = 1;
		senddata.data_info[senddata.num].sAddr.devID = 1;
		senddata.data_info[senddata.num].sAddr.typeID = 2;
		senddata.data_info[senddata.num].data_size = 4;
		senddata.data_info[senddata.num].el_tag = _FLOAT_;
		senddata.data_info[senddata.num].sAddr.pointID = 4;
		*(float *)&senddata.data_info[senddata.num].data[0] = sum_pw;
		senddata.num++;
	}
	//	if(sum_dpw>0)
	{
		senddata.data_info[senddata.num].sAddr.portID = 1;
		senddata.data_info[senddata.num].sAddr.devID = 1;
		senddata.data_info[senddata.num].sAddr.typeID = 2;
		senddata.data_info[senddata.num].data_size = 4;
		senddata.data_info[senddata.num].el_tag = _FLOAT_;
		senddata.data_info[senddata.num].sAddr.pointID = 5;
		*(float *)&senddata.data_info[senddata.num].data[0] = sum_dpw;
		senddata.num++;
	}
	ret = sendtotask(&senddata);

	if (ret == 1)
	{
		printf("1充放电数据上传成功！！！\n");
	}
	else
		printf("2充放电数据上传成功失败！！！\n");

	return 0;
}

static int LcdTo61850_YX(LCD_YC_YX_DATA *pdata)
{
	LCD_YC_YX_DATA temp;
	temp = *(LCD_YC_YX_DATA *)pdata;
	MyData senddata;
	int i;
	int base = 12;
	int ret;
	printf("LcdTo61850_YX收到遥信数据  lcdid=%d  pcsid_lcd=%d sn=%d  \n", temp.lcdid, temp.pcsid, temp.sn);

	for (i = 0; i < temp.data_len; i++)
	{

		senddata.data_info[i].sAddr.portID = 3;
		senddata.data_info[i].sAddr.devID = temp.lcdid * 6 + temp.pcsid;
		senddata.data_info[i].sAddr.typeID = 2;
		senddata.data_info[i].data_size = 2;
		senddata.data_info[i].el_tag = _U_SHORT_;
		senddata.data_info[i].sAddr.pointID = base + i;
		senddata.data_info[i].data[0] = temp.pcs_data[i] / 256;
		senddata.data_info[i].data[1] = temp.pcs_data[i] % 256;
	}
	senddata.num = temp.data_len;
	ret = sendtotask(&senddata);

	if (ret == 1)
	{
		printf("遥信实时数据上传成功！！！\n");
	}
	else
		printf("遥信实时数据上传成功失败！！！\n");

	return 0;
}
int recvfromlcd(unsigned char type, void *pdata)
{
	switch (type)
	{
	case _YC_:
	{
		// int Apparent_power;
		LCD_YC_YX_DATA temp;
	    static unsigned char flag_recv_pcs[] = {0, 0, 0, 0, 0, 0};
	    static int flag_recv_lcd = 0;
		temp = *(LCD_YC_YX_DATA *)pdata;
		yc_data[temp.sn] = temp;


		printf("收到遥测数据  sn=%d  lcdid=%d  pcsid_lcd=%d\n", temp.sn, temp.lcdid, temp.pcsid);
		// Apparent_power=Yc_PW_Data[temp.sn-1];
		// if (pcs_fault_flag[temp.sn] == 0)
		{
			LcdTo61850_YC(temp.lcdid, temp.pcsid, temp.pcs_data);
			// fun_realtime//上传实时数据
		}
	flag_recv_pcs[temp.lcdid] |= (1 << (temp.pcsid - 1));

	if (flag_recv_pcs[temp.lcdid] == flag_RecvNeed_PCS[temp.lcdid])
		{

		flag_recv_lcd |= (1 << temp.lcdid);
		flag_recv_pcs[temp.lcdid] = 0;


		}

	if (flag_recv_lcd == pFrome61850->flag_RecvNeed_LCD)//上传平均值和总和值
	{


					printf("上传平均值或总和值 flag_recv=%x\n", flag_recv_lcd);
			countSumAve_yc_Send();
		flag_recv_lcd = 0;
	}


	}
	break;
	case _YX_:
	{
		printf("recvfromlcd收到遥信数据\n");
		LcdTo61850_YX((LCD_YC_YX_DATA *)pdata);

		// unsigned char b;
		//  b = temp.pcs_data[0];
		//  if (b & (1 << bPcsRunning))
		//  {
		//  	setbit(Yx_Pcs_Status, (32 - temp.sn));
		//  	//			Yx_Pcs_Status |= (1<<(32-temp.sn));
		//  }
		//  else
		//  {
		//  	clrbit(Yx_Pcs_Status, (32 - temp.sn));
		//  }
	}

	case _ZJYX_:
	{
		printf("接收到整机遥信！！！！\n");
	}
	break;

	case _ZJYC_:
	{

		LCD_YC_YX_DATA temp;
		static unsigned int flag_recv = 0;
		temp = *(LCD_YC_YX_DATA *)pdata;
		flag_recv |= (1 << (temp.lcdid - 1));
		zjyc_data[temp.lcdid - 1] = temp;
		// if (flag_recv == g_flag_RecvNeed_LCD)
		// {
		// }

		printf("接收到整机遥测！！！！\n");
	}
		// case _PARA_:
		// {

		// }
		// break;

	default:
		break;
	}
	return 0;
}

// typedef int (*CallbackYK)(unsigned char, void *pdata);			  //遥控回调函数签名

// typedef int (*yk_fun)(unsigned char, YK_PARA *, CallbackYK pfun); //命令处理函数指针

void subscribeFromLcd(void)
{

	void *handle;
	char *error;
#define LIB_LCD_PATH "/usr/lib/libmodtcp.so"
	typedef int (*outData2Other)(unsigned char, void *);		   //输出数据
	typedef int (*in_fun)(unsigned char type, outData2Other pfun); //命令处理函数指针

	in_fun my_func = NULL;
	printf("订阅LCD数据！！！！\n");
	//打开动态链接库

	handle = dlopen(LIB_LCD_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	dlerror();

	*(void **)(&my_func) = dlsym(handle, "SubscribeLcdData");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	*(void **)(&ykOrder_61850) = dlsym(handle, "ykOrderFromBms");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	printf("1订阅LCD数据！！！！\n");
	my_func(_YC_, recvfromlcd);
	my_func(_YX_, recvfromlcd);
	// my_func(_ZJYC_, recvfromlcd);
}
