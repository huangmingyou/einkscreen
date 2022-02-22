#include "../lib/Config/DEV_Config.h"
#include "example.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include "mongoose.h"

#include <math.h>
#include <time.h>

#include <stdlib.h>     //exit()
#include <signal.h>     //signal()

#define Enhance false

#define USE_Factory_Test false

#define USE_Normal_Demo true

#define USE_Touch_Panel false

UWORD VCOM = 1420;
g_Width=1872;
g_Height=1404;

IT8951_Dev_Info Dev_Info;
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 1;	//0: no rotate, no mirror
					//1: no rotate, horizontal mirror, for 10.3inch
					//2: no totate, horizontal mirror, for 5.17inch
					//3: no rotate, no mirror, isColor, for 6inch color
					
static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
char name[200];
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/upload")) {
	
      mg_http_upload(c, hm, &mg_fs_posix, "/tmp");
mg_http_get_var(&hm->query, "name", name, sizeof(name));
    } else {
      struct mg_http_serve_opts opts = {.root_dir = "web_root"};
      mg_http_serve_dir(c, hm, &opts);
    }
printf("文件名:%s\n",name);
  }
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_ACCEPT && fn_data != NULL) {
    struct mg_tls_opts opts = {
        //.ca = "ca.pem",         // Uncomment to enable two-way SSL
        .cert = "server.pem",     // Certificate PEM file
        .certkey = "server.pem",  // This pem contains both cert and key
    };
    mg_tls_init(c, &opts);
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/api/f1")) {
    	Display_BMP_Example(g_Width, g_Height, Init_Target_Memory_Addr, 4,hm->body.ptr);
	printf("%s\n",hm->body.ptr);
      mg_http_reply(c, 200, "", "{\"result\": %d}\n", 123);  // Serve REST
    } else if (mg_http_match_uri(hm, "/api/f2/*")) {
      mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int) hm->uri.len,
                    hm->uri.ptr);
    } else {
      struct mg_http_serve_opts opts = {.root_dir = "/tmp/"};
      mg_http_serve_dir(c, ev_data, &opts);
    }
  }
  (void) fn_data;
}


void  Handler(int signo){
    Debug("\r\nHandler:exit\r\n");
    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf = NULL;
    }
    if(Panel_Frame_Buf != NULL){
        free(Panel_Frame_Buf);
        Debug("free Panel_Frame_Buf\r\n");
        Panel_Frame_Buf = NULL;
    }
    if(Panel_Area_Frame_Buf != NULL){
        free(Panel_Area_Frame_Buf);
        Debug("free Panel_Area_Frame_Buf\r\n");
        Panel_Area_Frame_Buf = NULL;
    }
    if(bmp_src_buf != NULL){
        free(bmp_src_buf);
        Debug("free bmp_src_buf\r\n");
        bmp_src_buf = NULL;
    }
    if(bmp_dst_buf != NULL){
        free(bmp_dst_buf);
        Debug("free bmp_dst_buf\r\n");
        bmp_dst_buf = NULL;
    }
    Debug("Going to sleep\r\n");
    EPD_IT8951_Sleep();
    DEV_Module_Exit();
    exit(0);
}


int main(int argc, char *argv[])
{
clock_t begin = clock();
    //Exception handling:ctrl + c
    signal(SIGINT, Handler);

    if (argc < 2){
        Debug("Please input VCOM value on FPC cable!\r\n");
        Debug("Example: sudo ./epd -2.51\r\n");
        exit(1);
    }

    //Init the BCM2835 Device
    if(DEV_Module_Init()!=0){
        return -1;
    }

    double temp;
    sscanf(argv[1],"%lf",&temp);
    VCOM = (UWORD)(fabs(temp)*1000);
    Debug("VCOM value:%d\r\n", VCOM);
	sscanf(argv[2],"%d",&epd_mode);
    Debug("Display mode:%d\r\n", epd_mode);

    Dev_Info = EPD_IT8951_Init(VCOM);


clock_t end = clock();
	double time_consumption = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("初始化时间花费: %f\n", time_consumption);


#if(Enhance)
    Debug("Attention! Enhanced driving ability, only used when the screen is blurred\r\n");
    Enhance_Driving_Capability();
    printf("run this");
#endif

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    char* LUT_Version = (char*)Dev_Info.LUT_Version;
    if( strcmp(LUT_Version, "M641") == 0 ){
        //6inch e-Paper HAT(800,600), 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 4;
        Four_Byte_Align = true;
    }else if( strcmp(LUT_Version, "M841_TFAB512") == 0 ){
        //Another firmware version for 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 6;
        Four_Byte_Align = true;
    }else if( strcmp(LUT_Version, "M841") == 0 ){
        //9.7inch e-Paper HAT(1200,825)
        A2_Mode = 6;
    }else if( strcmp(LUT_Version, "M841_TFA2812") == 0 ){
        //7.8inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }else if( strcmp(LUT_Version, "M841_TFA5210") == 0 ){
        //10.3inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }else{
        //default set to 6 as A2 Mode
        A2_Mode = 6;
    }
    Debug("A2 Mode:%d\r\n", A2_Mode);

clock_t t_r1 = clock();
//	EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
clock_t t_r2 = clock();
	time_consumption = (double)(t_r2 - t_r1) / CLOCKS_PER_SEC;
	printf("刷新时间花费: %f\n", time_consumption);

#if(USE_Factory_Test)
	if(epd_mode == 3) 	// Color Test
		Color_Test(Dev_Info, Init_Target_Memory_Addr);
    else				// Normal Test
		Factory_Test_Only(Dev_Info, Init_Target_Memory_Addr);
#endif

// web server

  struct mg_mgr mgr;
  struct mg_timer t1;

  mg_mgr_init(&mgr);
  mg_log_set("3");      
  mg_http_listen(&mgr, "http://0.0.0.0:8000", cb, NULL);
  mg_http_listen(&mgr, "http://0.0.0.0:8080", fn, NULL);
                                        
  for (;;) mg_mgr_poll(&mgr, 50);       
  mg_mgr_free(&mgr);                    




/*
#if(USE_Normal_Demo)
	t_r1 = clock();
    	Display_BMP_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4,argv[3]);
	t_r2 = clock();
	time_consumption = (double)(t_r2 - t_r1) / CLOCKS_PER_SEC;
	printf("显示图片时间花费: %f\n", time_consumption);
#endif
*/



    DEV_Module_Exit();
    return 0;
}
