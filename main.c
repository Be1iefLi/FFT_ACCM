#include<stdio.h>
#include<complex.h>
#include<stdlib.h>
#include<math.h>
#include"cosinint.h"

#define voice_number 499
#define window_time 0.016
#define sample_rate 8000
#define hop_time 0.016
extern int cosint_list_0[31];
extern int cosint_list_1[31];
extern int cosint_list_2[31];
extern int cosint_list_3[31];

int main();
int cosint_list(int num, int cos_num);
int sinint_list(int num, int sin_num);
int data_readin(int *sample, int num);
int* pre_emphasizing(int *sample,int len);
int* Hamming(int hamming_window_length);
int* Framing(int* sample, int *pframe_num, int *pframe_sample_length, int len, int frame_size);
void BF2I_LOW(int* dataR,int* dataI,int N,int pos);
void BF2I_HIGH(int* dataR,int* dataI,int N,int pos);
void BF2I_LOW(int* dataR,int* dataI,int N,int pos);
void BF2I_HIGH(int* dataR,int* dataI,int N,int pos); 
void FFT(int *frame_sample, int *FFT_sample_R, int *FFT_sample_I, int frame_size, int pos);
void write_to_file(int *FFT_sample_R, int *FFT_sample_I, int frame_sample_length, int num);
void compute_error(int *FFT_sample, int frame_sample_length, int num);

/*
Function Name: cosint_list
Input:
int num @ The position of cosint_list @ cosint_list的位置
int cos_num @ The number of cosint_list @ cosint_list的编号
Output:
int cos_value @ The corresponding cosint_list value @ 相应的cosint_list数值
*/
int cosint_list(int num, int cos_num){
    int *cosint_list_original;
    switch (cos_num)
    {
    case 0:
        cosint_list_original = cosint_list_0;
        break;
    case 1:
        cosint_list_original = cosint_list_1;
        break;
    case 2:
        cosint_list_original = cosint_list_2;
        break;
    case 3:
        cosint_list_original = cosint_list_3;
        break;
    default:
        cosint_list_original = cosint_list_0;
        break;
    }
    while(num < 0){
        num += 128;
    }
    while(num > 128){
        num -= 128;
    }
    if(num == 0 || num == 128){
        return 32768;//0x8000
    }
    if(num == 32 || num == 96){
        return 0;
    }
    if(num == 64){
        return -32768;//- 0x8000
    }
    if(num < 32){
        return cosint_list_original[num - 1];
    }
    if(num > 96){
        return cosint_list_original[127 - num];
    }
    if(num > 32 && num < 64){
        return -cosint_list_original[63 - num];
    }
    if(num > 64){
        return -cosint_list_original[num - 65];
    }
}

/*
Function Name: sinint_list
Input:
int num @ The position of sinint_list @ sinint_list的位置
int sin_num @ The number of sinint_list @ sinint_list的编号
Output:
int sin_value @ The corresponding sinint_list value @ 相应的sinint_list数值
*/
int sinint_list(int num, int sin_num){
    return cosint_list(32 - num, sin_num);
}

/*
Function Name: data_readin
Input:
int num @ The number of the readin data file @ 读入数据文件的编号
Output:
int len @ The length of sample @ sample的长度
int **sample @ The sample pointer for readin data @ 为读入数据准备的sample指针
*/
int data_readin(int *sample, int num){
    FILE *filedata = NULL;
    char data_name[100];
    int i;

    sprintf(data_name, "%s%d%s", "data/", num, ".txt");
    printf("Data Readin: %s\n", data_name);
    filedata = fopen(data_name,"r");

    for(i = 0; i < 4000; i++){
        sample[i] = 0;
    }

    int len = 0;

    while (len < 4000)
    {
        fscanf(filedata, "%d", sample + len);
        len++;
    }
    fclose(filedata);

    return len;
}

/*
Function Name: pre_emphasized
Input:
int *sample @ Original data from file @ 从文件中获得的原始数据
int len @ The length of sample @ sample的长度
Output:
int *emphasized_sample @ The sample which has been pre_emphasized @ 预加重后的sample
*/
int* pre_emphasizing(int *sample,int len){
    int *emphasized_sample = NULL;
	emphasized_sample = (int *)malloc(len * sizeof(int));
	emphasized_sample[0] = sample[0];
	int i;
	for(i = 1; i < len; i++)
	{
		emphasized_sample[i] = (sample[i] - sample[i-1]) + (sample[i-1]>>4); 
	}
	return emphasized_sample;
}

/*
Function Name: Hamming
Input:
int hamming_window_length @ The length of the hamming window @ hamming窗的长度
Output:
int *hamming_window @ Hamming window @ hamming窗
*/
int* Hamming(int hamming_window_length){
    int *hamming_window;
    int temp = 0;
	int i;
    hamming_window = (int *) malloc ((hamming_window_length) * sizeof(int));
	for (i = 0; i < hamming_window_length; i++)
	{
		temp = round((double)i / ((double)hamming_window_length) * 128);
		hamming_window[i] = 16384 - (cosint_list(temp, 0)>>1);
	}
    return hamming_window;
}

/*
Function Name: Framing
Input:
int *sample @ Sample for framing @ 需要分帧的sample
int len @ The length of sample @ sample长度
int frame_size @ The size of frame @ 帧大小
Output:
int *pframe_num @ The number of frame @ 分帧后的帧数
int *pframe_sample_length @ The length of framed sample(frame_sample) @ 分帧后的sample长度
int *frame_sample @ The framed sample @ 分帧后的sample
*/
int* Framing(int *sample, int *pframe_num, int *pframe_sample_length, int len, int frame_size){
    int i, j;

    int *hamming_window = NULL;
    hamming_window = Hamming(frame_size);

    int hop_step = hop_time * sample_rate;
    *pframe_num = ceil(((double)(len)) / hop_step);
    *pframe_sample_length = *pframe_num * frame_size;
    printf("frame_sample_length = %d\n", *pframe_sample_length);
    int *frame_sample = (int *) malloc ((*pframe_sample_length) * sizeof(int));
    for (i = 0; i < *pframe_sample_length; i++)
        frame_sample[i] = 0;
    
    FILE *fileFrame = NULL;
    fileFrame = fopen("Frame.txt","w");

    for(i = 0; i * hop_step < len; i++){
        for(j = 0; j < frame_size; j++){
            if(j < frame_size && i * hop_step + j < len){
                frame_sample[i * frame_size + j] = (sample[i * hop_step + j] * hamming_window[j]) >> 15;
            }
            else
                frame_sample[i * frame_size + j] = 0;
        }
    }

    for(i = 0; i < *pframe_sample_length; i++){
		fprintf(fileFrame, "%d\n", frame_sample[i]);
	}
    fclose(fileFrame);

    return frame_sample;
}

/*
Function Name: BF2I_LOW
Input:
int *dataR @ The real number of data @ 数据的实数部分
int *dataI @ The imaginary number of data @ 数据的虚数部分
int N @ The length of data @ 数据的长度
int pos @ The position of the data to treat @ 需要处理的数据位置
*/
void BF2I_LOW(int* dataR,int* dataI,int N,int pos)
{
	int n=0;
	int temp1,temp2,temp3,temp4;
    for(n = 0 ; n<N/2 ; n++)
    {
        temp1 = dataR[n+pos] + dataR[n+N/2+pos];
		temp2 = dataI[n+pos] + dataI[n+N/2+pos];
		temp3 = dataR[n+pos] - dataR[n+N/2+pos];
		temp4 = dataI[n+pos] - dataI[n+N/2+pos];
		dataR[n+pos] = temp1;
        dataI[n+pos] = temp2;
        dataR[n+N/2+pos] = temp3;
        dataI[n+N/2+pos] = temp4;
    }
}

/*
Function Name: BF2I_HIGH
Input:
int *dataR @ The real number of data @ 数据的实数部分
int *dataI @ The imaginary number of data @ 数据的虚数部分
int N @ The length of data @ 数据的长度
int pos @ The position of the data to treat @ 需要处理的数据位置
*/
void BF2I_HIGH(int* dataR,int* dataI,int N,int pos)
{
	int n=0;
	int temp1,temp2,temp3,temp4;
    for(n = 0 ; n<N/2 ; n++)
    {
        temp1 = (dataR[n+pos] + dataR[n+N/2+pos])>>1;
		temp2 = (dataI[n+pos] + dataI[n+N/2+pos])>>1;
		temp3 = (dataR[n+pos] - dataR[n+N/2+pos])>>1;
		temp4 = (dataI[n+pos] - dataI[n+N/2+pos])>>1;
		dataR[n+pos] = temp1;
        dataI[n+pos] = temp2;
        dataR[n+N/2+pos] = temp3;
        dataI[n+N/2+pos] = temp4;
    }
}

/*
Function Name: BF2II_LOW
Input:
int *dataR @ The real number of data @ 数据的实数部分
int *dataI @ The imaginary number of data @ 数据的虚数部分
int N @ The length of data @ 数据的长度
int pos @ The position of the data to treat @ 需要处理的数据位置
*/
void BF2II_LOW(int* dataR,int* dataI,int N,int pos)
{   
    int n2 = 0;
    int n1 = 0;
    int n = 0;
    int temp1,temp2,temp3,temp4;
    for(n2 = 0 ; n2<2 ; n2++)
    {
        for(n1 = 0 ; n1<N/4 ; n1++)
        {
            n = n1 + n2*N/2;
            if(n2==0)
            {
                temp1 = dataR[n+pos] + dataR[n+N/4+pos];
				temp2 = dataI[n+pos] + dataI[n+N/4+pos];
				temp3 = dataR[n+pos] - dataR[n+N/4+pos];
				temp4 = dataI[n+pos] - dataI[n+N/4+pos];
				dataR[n+pos] = temp1;
                dataI[n+pos] = temp2;
                dataR[n+N/4+pos] = temp3;
                dataI[n+N/4+pos] = temp4;
            }
            else
            {
            	temp1 = dataR[n+pos] + dataI[n+N/4+pos];
            	temp2 = dataI[n+pos] - dataR[n+N/4+pos];
            	temp3 = dataR[n+pos] - dataI[n+N/4+pos];
            	temp4 = dataI[n+pos] + dataR[n+N/4+pos];
                dataR[n+pos] = temp1;
                dataI[n+pos] = temp2;
                dataR[n+N/4+pos] = temp3;
                dataI[n+N/4+pos] = temp4;
            }
        }
    }
}

/*
Function Name: BF2II_HIGH
Input:
int *dataR @ The real number of data @ 数据的实数部分
int *dataI @ The imaginary number of data @ 数据的虚数部分
int N @ The length of data @ 数据的长度
int pos @ The position of the data to treat @ 需要处理的数据位置
*/
void BF2II_HIGH(int* dataR,int* dataI,int N,int pos)
{   
    int n2 = 0;
    int n1 = 0;
    int n = 0;
    int temp1,temp2,temp3,temp4;
    for(n2 = 0 ; n2<2 ; n2++)
    {
        for(n1 = 0 ; n1<N/4 ; n1++)
        {
            n = n1 + n2*N/2;
            if(n2==0)
            {
                temp1 = (dataR[n+pos] + dataR[n+N/4+pos])>>1;
				temp2 = (dataI[n+pos] + dataI[n+N/4+pos])>>1;
				temp3 = (dataR[n+pos] - dataR[n+N/4+pos])>>1;
				temp4 = (dataI[n+pos] - dataI[n+N/4+pos])>>1;
				dataR[n+pos] = temp1;
                dataI[n+pos] = temp2;
                dataR[n+N/4+pos] = temp3;
                dataI[n+N/4+pos] = temp4;
            }
            else
            {
            	temp1 = (dataR[n+pos] + dataI[n+N/4+pos])>>1;
            	temp2 = (dataI[n+pos] - dataR[n+N/4+pos])>>1;
            	temp3 = (dataR[n+pos] - dataI[n+N/4+pos])>>1;
            	temp4 = (dataI[n+pos] + dataR[n+N/4+pos])>>1;
                dataR[n+pos] = temp1;
                dataI[n+pos] = temp2;
                dataR[n+N/4+pos] = temp3;
                dataI[n+N/4+pos] = temp4;
            }
        }
    }
}

/*
Function Name: twiddle
Input:
int *dataR @ The real number of data @ 数据的实数部分
int *dataI @ The imaginary number of data @ 数据的虚数部分
int N @ The length of data @ 数据的长度
int pos @ The position of the data to treat @ 需要处理的数据位置
int layer @ The layer of FFT @ FFT的层数
*/
void twiddle(int* dataR, int* dataI, int N, int pos, int layer)
{
    int n=0;
    int i=0,t;
    int twiddleR=0,twiddleI=0;
    int dataR_temp,dataI_temp;
    int add1=0,add2=0,add3=0,add4=0;
    int k[4]={0,2,1,3};
    for(i=0;i<4;i++)
    {
        for(n=0;n<N/4;n++)
        {
            t=(n*k[i]*128)/N;
			twiddleR=cosint_list(t, layer);
			twiddleI=sinint_list(t, layer);
            add1 = (dataR[i*N/4+n+pos]*twiddleR)>>15;
            add2 = (dataI[i*N/4+n+pos]*twiddleI)>>15;
            add3 = (dataI[i*N/4+n+pos]*twiddleR)>>15;
            add4 = (dataR[i*N/4+n+pos]*twiddleI)>>15;
            dataR_temp = add1+add2;
            dataI_temp = add3-add4;
            dataR[i*N/4+n+pos]=dataR_temp;
            dataI[i*N/4+n+pos]=dataI_temp;
        }    			
	}	
}

/*
Function Name: FFT
Input:
int *frame_sample @ The sample after framing @ 分帧后的sample
int frame_size @ The size of frame @ 分帧后的帧数
int pos @ The position of the data for FFT @ 需要FFT处理的数据位置
Output:
int *FFT_sample_R @ The real part of data after FFT @ FFT处理后的数据的实数部分
int *FFT_sample_I @ The imaginary part of data after FFT @ FFT处理后的数据的虚数部分
*/
void FFT(int *frame_sample, int *FFT_sample_R, int *FFT_sample_I, int frame_size, int pos){
    int *dataR = (int *) malloc (frame_size * sizeof(int));
    int *dataI = (int *) malloc (frame_size * sizeof(int));
    int *R_temp = (int *) malloc (frame_size * sizeof(int));
    int *I_temp = (int *) malloc (frame_size * sizeof(int));
    int i, j;
    int x0, x1, x2, x3, x4, x5, x6, xx;

    for(i = 0; i < frame_size; i++){
        dataR[i] = frame_sample[i + pos];
        dataI[i] = 0;
    }

    int N = frame_size; //128
    BF2I_LOW(dataR, dataI, N, 0);  //64
    BF2II_HIGH(dataR, dataI, N, 0);  //32
    twiddle(dataR, dataI, N, 0, 1);

    N=N / 4; //N=32
    for(i=0;i<4;i++)
    {
        BF2I_HIGH(dataR, dataI, N, i*N);//16
        BF2II_LOW(dataR, dataI, N, i*N);//8       
        twiddle(dataR, dataI, N, i*N, 2);
    }   

    N = N / 4;  //N=8
    for(i=0;i<16;i++)
    {
        BF2I_HIGH(dataR, dataI, N, i*N);//4
        BF2II_LOW(dataR, dataI, N, i*N);//2
        twiddle(dataR, dataI, N, i*N, 3);
    }

    N = N / 4;
    for(i=0;i<64;i++)
    {
        BF2I_HIGH(dataR,dataI,N,i*N);//1
    }

    for(i = 0; i < frame_size; i++)
	{
		x0 = x1 = x2 = x3 = x4 = x5 = x6 = 0;
		x0 = i & 0x01; x1 = (i / 2) & 0x01; x2 = (i / 4) & 0x01; x3 = (i / 8) & 0x01; x4 = (i / 16) & 0x01; 
		x5 = (i / 32) & 0x01; x6 = (i / 64) & 0x01;// x7 = (i / 128) & 0x01; //x8 = (i / 256) & 0x01;
		xx = x0 * 64 + x1 * 32 + x2 * 16 + x3 * 8 + x4 * 4 + x5 * 2 + x6;//+ x8
		R_temp[xx]= dataR[i];
		I_temp[xx]= dataI[i];	 
	}

    for(i = 0; i < frame_size; i++)
	{ 
	    FFT_sample_R[i + pos] = R_temp[i];
        FFT_sample_I[i + pos] = I_temp[i];
	}


    free(dataR);
    free(dataI);
    free(R_temp);
    free(I_temp);
}

/*
Function Name: write_to_file
Input:
int *FFT_sample_R @ The real part of data after FFT @ FFT处理后的数据的实数部分
int *FFT_sample_I @ The imaginary part of data after FFT @ FFT处理后的数据的虚数部分
int frame_sample_length @ the length of FFT_sample @ FFT_sample的长度
int num @ The number of output file @ 输出文件的序号
*/
void write_to_file(int *FFT_sample_R, int *FFT_sample_I, int frame_sample_length, int num){
    int i;
    FILE *file_FFT_R = NULL, *file_FFT_I = NULL;
    char data_name[100];
    sprintf(data_name, "%s%d%s", "fft/", num, "r.txt");
    file_FFT_R = fopen(data_name,"w");
    sprintf(data_name, "%s%d%s", "fft/", num, "i.txt");
    file_FFT_I = fopen(data_name,"w");
    for(i = 0; i < frame_sample_length; i++){
        fprintf(file_FFT_R, "%d\n", FFT_sample_R[i]);
        fprintf(file_FFT_I, "%d\n", FFT_sample_I[i]);
    }
    fclose(file_FFT_R);
    fclose(file_FFT_I);
}

/*
Function Name: computer_error
Input:
int *FFT_sample_R @ The real part of data after FFT @ FFT处理后的数据的实数部分
int *FFT_sample_I @ The imaginary part of data after FFT @ FFT处理后的数据的虚数部分
int frame_sample_length @ the length of FFT_sample @ FFT_sample的长度
int num @ The number of fft_original @ fft_original的序号
*/
/*
void compute_error(int *FFT_sample_R, int *FFT_sample_I, int frame_sample_length, int num){
    int i;
    long double error_sum = 0, sum = 0;
    FILE *file_original_R = NULL, *file_original_I = NULL;
    char data_name[100];
    sprintf(data_name, "%s%d%s", "fft_original/", num, "r.txt");
    file_original_R = fopen(data_name,"r");
    sprintf(data_name, "%s%d%s", "fft_original/", num, "i.txt");
    file_original_I = fopen(data_name, "r");

    int FFT_sample_original_data_R, FFT_sample_original_data_I;
    for(i = 0; i < frame_sample_length; i++){
        fscanf(file_original_R, "%d", &FFT_sample_original_data_R);
        fscanf(file_original_I, "%d", &FFT_sample_original_data_I);
        sum += FFT_sample_original_data;
        error_sum += pow(FFT_sample[i] - FFT_sample_original_data, 2);
    }
    fclose(file_original);

    double error_mean = error_sum / (double)frame_sample_length;
    double mean = sum / (double) frame_sample_length;

    double error_ratio = sqrt(error_mean) / mean * 100;
        
    printf("Error ratio of %d: %f %%\n", num, error_ratio);
}*/


int main(){
    int len = 0, i = 0, j = 0;
    int data_num;
    scanf("%d", &data_num);
    for(j = 0; j < data_num + 1; j++){
        int *sample = NULL, *emphasized_sample = NULL, *hamming_window = NULL;
        
        sample = (int *) malloc ((4000) * sizeof(int));
        len = data_readin(sample, j);

        emphasized_sample = pre_emphasizing(sample, len);

        int frame_size = (int)pow(2, ceil( log(window_time * sample_rate) / log(2.0) )); //128
        printf("frame_size = %d\n", frame_size);

        int frame_sample_length, frame_num;
        int *frame_sample = NULL;

        frame_sample = Framing(emphasized_sample, &frame_num, &frame_sample_length, len, frame_size);

        int *FFT_sample_R = (int *) malloc ((frame_sample_length) * sizeof(int));
        int *FFT_sample_I = (int *) malloc ((frame_sample_length) * sizeof(int));
        for(i = 0; i <frame_sample_length; i++){
            FFT_sample_R[i] = 0;
            FFT_sample_I[i] = 0;
        }
        for(i = 0; i < frame_num; i++)
            FFT(frame_sample, FFT_sample_R, FFT_sample_I, frame_size, i * frame_size);
        
        write_to_file(FFT_sample_R, FFT_sample_I, frame_sample_length, j);

        //compute_error(FFT_sample, frame_sample_length, j);

        free(sample);
        free(emphasized_sample);
        free(frame_sample);
        free(FFT_sample_R);
        free(FFT_sample_I);
    }

    return 0;
}
