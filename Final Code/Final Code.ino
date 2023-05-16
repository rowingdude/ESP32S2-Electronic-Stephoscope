#include <Esp.h>
#include "esp_adc_cal.h"
#include "driver/adc.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <ESP32WebServer.h>   //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include "CSS.h"
#include <ESPmDNS.h>
#include <soc/adc_channel.h>
#include <driver/dac.h>
#include <soc/dac_channel.h>

void SD_init(void);
void ADC_init(void);
static void check_efuse(void);
void SD_file_download(String filename);
void printDirectory(File dir, int numTabs);
String file_size(int bytes);
void File_Upload();
void handleFileUpload();
void SD_dir();
void SD_file_delete(String filename); 
void SD_file_delete(String filename);
void SendHTML_Header();
void SendHTML_Content();
void SendHTML_Stop();
void ReportSDNotPresent();
void ReportFileNotPresent(String target);
void ReportCouldNotCreateFile(String target);
String file_size(int bytes);
void Delay_CC(uint32_t ClockCycles);
void Record_data (void);


ESP32WebServer server(80);

SPIClass SPI1(FSPI);

#define servername "Ascalutation"
String Create = "";
String Delete= "";
String Name = "";
File UploadFile;
bool SD_present;
static esp_adc_cal_characteristics_t *adc_chars;

const int ChipSelect = 10;

#define SAMPLES_BUFFER_SIZE 1024 // Define buffer size for ADC samples (adjust as needed)

static uint16_t adc_buffer[SAMPLES_BUFFER_SIZE];  // Define buffer to store ADC samples


void SD_init(void){
  if (!SD.begin(SS)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
    SD_present = true;
  }
 
  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.cardType()) {
    case CARD_NONE:
      Serial.println("NONE");
      break;
    case CARD_MMC:
      Serial.println("MMC");
      break;
    case CARD_SD:
      Serial.println("SD");
      break;
    case CARD_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }


}

void WriteSelect(bool a2,bool a1, bool a0){
  digitalWrite(39, 1);
  digitalWrite(40, a2);
  digitalWrite(41, a1);
  digitalWrite(42, a0);

}



void ADC_init(void)
{
   
    check_efuse();
    //adc_digi_initialize(adc_digi_configuration);
    adc1_config_channel_atten(ADC1_CHANNEL_9, ADC_ATTEN_DB_11);
    vTaskDelay(20);
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
	  if (adc_chars == NULL) {
		  ESP_LOGE(TAG, "ADC INIT: Mem error");
		  return;
	  }
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_13, 0, adc_chars);
}


void initDMA() {
  dma_channel_attr_t dma_channel_attr;
  dma_channel_attr.attach_src = (void *)&ADC1->data;
  dma_channel_attr.attach_dst = (void *)adc_buffer;
  dma_channel_attr.mode = DMA_CHANNEL_MODE_NORMAL;
  dma_channel_attr.transfer_size = ADC_SAMPLE_NUM * 2;
  dma_channel_attr.src_addr_increment = 0; 
  dma_channel_attr.dst_addr_increment = 2;
  dma_channel_attr.src_endian = DMA_CHANNEL_ENDIAN_DISABLE;
  dma_channel_attr.dst_endian = DMA_CHANNEL_ENDIAN_DISABLE;
  dmaAttachPeriph(DMA_CHANNEL_0, DMA_PERIPH_ADC1); 
  dmaSetupTransfer(DMA_CHANNEL_0, &dma_channel_attr, 1);
  dmaStart(DMA_CHANNEL_0);
}

void loop() {
  // put your main code here, to run repeatedly:
  //if (Flag = true){
    //Recording_data();

 //}
  //else{
    ///server.handleClient();
  //}
}

uint32_t voltage1 = 0;
uint32_t adc_reading1 = 0;
uint32_t voltage2 = 0;
uint32_t adc_reading2 = 0;
uint32_t voltage3 = 0;
uint32_t adc_reading3 = 0;
uint32_t voltage4 = 0;
uint32_t adc_reading4 = 0;
uint32_t voltage5 = 0;
uint32_t adc_reading5 = 0;
uint32_t voltage6 = 0;
uint32_t adc_reading6 = 0;
uint32_t voltage7 = 0;
uint32_t adc_reading7 = 0;
uint32_t voltage8 = 0;
uint32_t adc_reading8 = 0;

void Record_data (void){
    //could have if RTX data is avaliable ove the wifi AP point use that 
    uint32_t Diff_1;
    File dir = SD.open("/");
    String FileName = "/Lung_Auscultation_Data.csv";
    int count = 0;
    while(1) {
      String Entry_Num = String(count);
      File entry = dir.openNextFile();
      String name = entry.name();
      if (!entry) {
        FileName = "/Lung_Auscultation_Data_" + Entry_Num + ".csv" ;
        Serial.println(FileName);
        break;
      }
      if (name = "/Lung_Auscultation_Data_" + Entry_Num + ".csv" ){
        count = count + 1;
      }
    }
    File Recording = SD.open(FileName, FILE_WRITE);
    Recording.println("TimeDiff, 0, 7.9583E-6, 1.5917E-6, 2.3875E-6, 3.1833E-6, 3.9792E-6, 2.3875E-6, 5.5708E-6, 6.3667E-6");
    Recording.println("Seconds, Channel 1, Channel 2, Channel 3, Channel 4, Channel 5, Channel 6, Channel 7, Channel 8, Channel 9");
    char Sample[100];
    adc_digi_start();
    double Time = 0; //difference in time is accounted for in signal processing 
    for(int i = 1.6E6; i > 0; i--){
      WriteSelect(0,0,0);
      uint32_t StartTime = ESP.getCycleCount(); 
      adc_reading1 = adc1_get_raw(ADC1_CHANNEL_5, ADC_WIDTH_BIT_13, adc_buffer, SAMPLES_BUFFER_SIZE);
      uint32_t EndTime = ESP.getCycleCount(); 
      WriteSelect(0, 0, 1); //select channel 2 on the multiplxer
      adc_reading2 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars); //take reading
      WriteSelect(0, 1, 0); //select channel 3 on the multiplxer
      adc_reading3 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage3 = esp_adc_cal_raw_to_voltage(adc_reading3, adc_chars); //take reading
      WriteSelect(0, 1, 1); //select channel 4 on the multiplxer
      adc_reading4 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage4 = esp_adc_cal_raw_to_voltage(adc_reading4, adc_chars); //take reading
      WriteSelect(1, 0, 0); //select channel 4 on the multiplxer
      adc_reading5 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage5 = esp_adc_cal_raw_to_voltage(adc_reading5, adc_chars); //take reading
      WriteSelect(1, 0, 1); //select channel 4 on the multiplxer
      adc_reading6 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage6 = esp_adc_cal_raw_to_voltage(adc_reading5, adc_chars); //take reading
      WriteSelect(1, 1, 1); //select channel 4 on the multiplxer
      adc_reading7 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage7 = esp_adc_cal_raw_to_voltage(adc_reading5, adc_chars); //take reading
      WriteSelect(1, 1, 1); //select channel 4 on the multiplxer
      adc_reading8 = adc1_get_raw(ADC1_CHANNEL_5);
      voltage8 = esp_adc_cal_raw_to_voltage(adc_reading5, adc_chars); //take reading
      sprintf(Sample, "%f, %u, %u, %u, %u, %u, %u, %u, %u",Time ,voltage1, voltage2, voltage3, voltage4, voltage5, voltage6, voltage7, voltage8); 
      Recording.println(Sample); 
      Diff_1 = EndTime - StartTime;
      Serial.printf("Whole thing Clock Cycles: %u \n",Diff_1);
    }
    Serial.printf("Yes");
}

//
//void ADC_Init(void)
//{
//    check_efuse();
//    adc1_config_channel_atten(ADC1_CHANNEL_9, ADC_ATTEN_DB_11);
//    vTaskDelay(20);
//    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
//	  if (adc_chars == NULL) {
//		  ESP_LOGE(TAG, "ADC INIT: Mem error");
//		  return;
//	  }
//    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_13, 0, adc_chars);
//}

static void check_efuse(void)
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        ESP_LOGI(TAG, "eFuse Two Point: Supported");
    } else {
        ESP_LOGE(TAG, "Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.");
    }
}


void setup() {
  // put your setup code here, to run once:
  bool setCpuFrequencyMhz(240);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(39, OUTPUT);
  pinMode(42, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(40,OUTPUT);
  pinMode(ChipSelect, OUTPUT);
  digitalWrite(ChipSelect,HIGH); //dunno if we need this high
  SPI1.begin(12, 13, 11, -1); //going to have to change this for the sd card
  SPI1.setBitOrder(MSBFIRST); // MSB first bit order
  SPI1.setDataMode(SPI_MODE0); //SPI Mode 0
  SPI1.setClockDivider(SPI_CLOCK_DIV128);
  digitalWrite(ChipSelect,LOW);
  // send in the address and value via SPI:
  SPI1.transfer(B01000000); // Instruction Register set to "write to register"
  SPI1.transfer(B00000110); // Gain Register set to "Gain of +2"
  // take the SS pin high to de-select the chip:
  digitalWrite(ChipSelect,HIGH);
  Serial.print("\nInitialiazing ADC...");
  ADC_init();
  DMA_init();
  Serial.print("\nInitializing SD card...");
  SD_init();
  WiFi.softAP("ESP32S2", "123456789");
  if (!MDNS.begin(servername)) 
    {          
      Serial.println(F("Error setting up MDNS responder!")); 
      ESP.restart(); 
    } 
  
    /*********  Server Commands  **********/
  server.on("/",         SD_dir);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
  
  server.begin();
    
  Serial.println("HTTP server started");
  Record_data();
}



void SD_file_download(String filename)
{
  if (SD_present) 
  { 
    File download = SD.open("/"+filename);
    if (download) 
    {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}



void File_Upload()
{
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>"); 
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:25%' type='file' name='fupload' id = 'fupload' value=''>");
  webpage += F("<button class='buttons' style='width:10%' type='submit'>Upload File</button><br><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html",webpage);
}

void handleFileUpload()
{ 
  HTTPUpload& uploadfile = server.upload(); //See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            //For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SD.remove(filename);                         //Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(filename, FILE_WRITE);  //Open the file for writing in SD (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          //If the file was successfully created
    {                                    
      UploadFile.close();   //Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>"); 
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br><br>"; 
      webpage += F("<a href='/'>[Back]</a><br><br>");
      append_page_footer();
      server.send(200,"text/html",webpage);
    } 
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}


void SD_dir()
{
  if (SD_present) 
  {
    //Action acording to post, dowload or delete, by MC 2022
    if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
    { 
      Serial.println(server.arg(0));
  
      String Order = server.arg(0);
      Serial.println(Order);
      
      if (Order.indexOf("download_")>=0)
      {
        Order.remove(0,9);
        SD_file_download(Order);
        Serial.println(Order);
      }
  
      if ((server.arg(0)).indexOf("delete_")>=0)
      {
        Order.remove(0,7);
        SD_file_delete(Order);
        Serial.println(Order);
      }
    }

    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();    
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else 
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   //Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

//Prints the directory, it is called in void SD_dir() 
void printDirectory(const char * dirname, uint8_t levels)
{
  
  File root = SD.open(dirname);

  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();

  int i = 0;
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='download'"); 
      webpage += F("' value='"); webpage +="download_"+String(file.name()); webpage +=F("'>Download</button>");
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='delete'"); 
      webpage += F("' value='"); webpage +="delete_"+String(file.name()); webpage +=F("'>Delete</button>");
      webpage += "</td>";
      webpage += "</tr>";

    }
    file = root.openNextFile();
    i++;
  }
  file.close();

 
}

void Delay_CC(uint32_t ClockCycles){
  uint32_t StartCount = ESP.getCycleCount(); //prevent overflow errors
  while(ESP.getCycleCount() < StartCount + ClockCycles){
  }
}


void SD_file_delete(String filename) 
{ 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD.open("/"+filename, FILE_READ); //Now read data from SD Card 
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
} 




void SendHTML_Header()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); //Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Content
void SendHTML_Content()
{
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Stop
void SendHTML_Stop()
{
  server.sendContent("");
  server.client().stop(); //Stop is needed because no content length was sent
}


//ReportSDNotPresent
void ReportSDNotPresent()
{
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportFileNotPresent
void ReportFileNotPresent(String target)
{
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportCouldNotCreateFile
void ReportCouldNotCreateFile(String target)
{
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//File size conversion
String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}






