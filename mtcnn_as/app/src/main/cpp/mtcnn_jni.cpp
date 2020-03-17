#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <string>
#include <vector>

// ncnn
#include "net.h"

#include "mtcnn.h"
using namespace std;
#define TAG "MtcnnSo"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
static MTCNN *mtcnn;

//sdk是否初始化成功
bool detection_sdk_init_ok = false;




extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mtcnn_MTCNN_FaceDetectionModelInit(JNIEnv *env, jobject thiz,
                                                    jbyteArray det1_param, jbyteArray det1_bin,
                                                    jbyteArray det2_param, jbyteArray det2_bin,
                                                    jbyteArray det3_param, jbyteArray det3_bin) {
    // TODO: implement FaceDetectionModelInit()

    std::vector<ncnn::Mat> param_files;
    std::vector<ncnn::Mat> bin_files;

    ncnn::Mat ncnn_det1_param,ncnn_det2_param,ncnn_det3_param;
    ncnn::Mat ncnn_det1_bin,ncnn_det2_bin,ncnn_det3_bin;


    // init param1
    {
        int len = env->GetArrayLength(det1_param);
        ncnn_det1_param.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det1_param, 0, len, (jbyte *) ncnn_det1_param);
        param_files.push_back(ncnn_det1_param);

    }

    // init bin1
    {
        int len = env->GetArrayLength(det1_bin);
        ncnn_det1_bin.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det1_bin, 0, len, (jbyte *) ncnn_det1_bin);

        bin_files.push_back(ncnn_det1_bin);

    }


    // init param2
    {
        int len = env->GetArrayLength(det2_param);
        ncnn_det2_param.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det2_param, 0, len, (jbyte *) ncnn_det2_param);
        param_files.push_back(ncnn_det2_param);
    }

    // init bin2
    {
        int len = env->GetArrayLength(det2_bin);
        ncnn_det2_bin.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det2_bin, 0, len, (jbyte *) ncnn_det2_bin);
        bin_files.push_back(ncnn_det2_bin);
    }


    // init param3
    {
        int len = env->GetArrayLength(det3_param);
        ncnn_det3_param.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det3_param, 0, len, (jbyte *) ncnn_det3_param);
        param_files.push_back(ncnn_det3_param);
    }

    // init bin3
    {
        int len = env->GetArrayLength(det3_bin);
        ncnn_det3_bin.create(len, (size_t) 1u);
        env->GetByteArrayRegion(det3_bin, 0, len, (jbyte *) ncnn_det3_bin);
        bin_files.push_back(ncnn_det3_bin);
    }


    mtcnn = new MTCNN(param_files,bin_files);
    mtcnn->SetMinFace(1);


    detection_sdk_init_ok = true;
    jboolean tRet = true;
    return tRet;

}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_mtcnn_MTCNN_FaceDetect(JNIEnv *env, jobject thiz, jbyteArray image_date,
                                        jint image_width, jint image_height, jint image_channel) {
    // TODO: implement FaceDetect()
    //  LOGD("JNI开始检测人脸");
    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型SDK未初始化，直接返回空");
        return NULL;
    }

    int tImageDateLen = env->GetArrayLength(image_date);
    if(image_channel == tImageDateLen / image_width / image_height){
        LOGD("数据宽=%d,高=%d,通道=%d",image_width,image_height,image_channel);
    }
    else{
        LOGD("数据长宽高通道不匹配，直接返回空");
        return NULL;
    }

    jbyte *imageDate = env->GetByteArrayElements(image_date, NULL);
    if (NULL == imageDate){
        LOGD("导入数据为空，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    if(image_width<20||image_height<20){
        LOGD("导入数据的宽和高小于20，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    //TODO 通道需测试
    if(3 == image_channel || 4 == image_channel){
        //图像通道数只能是3或4；
    }else{
        LOGD("图像通道数只能是3或4，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    //int32_t minFaceSize=40;
    //mtcnn->SetMinFace(minFaceSize);

    unsigned char *faceImageCharDate = (unsigned char*)imageDate;
    ncnn::Mat ncnn_img;
    if(image_channel==3) {
        ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_BGR2RGB,
                                          image_width, image_height);
    }else{
        ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_RGBA2RGB, image_width, image_height);
    }

    std::vector<Bbox> finalBbox;
    mtcnn->detect(ncnn_img, finalBbox);

    int32_t num_face = static_cast<int32_t>(finalBbox.size());
    LOGD("检测到的人脸数目：%d\n", num_face);

    int out_size = 1+num_face*14;
    //  LOGD("内部人脸检测完成,开始导出数据");
    int *faceInfo = new int[out_size];
    faceInfo[0] = num_face;
    for(int i=0;i<num_face;i++){
        faceInfo[14*i+1] = finalBbox[i].x1;//left
        faceInfo[14*i+2] = finalBbox[i].y1;//top
        faceInfo[14*i+3] = finalBbox[i].x2;//right
        faceInfo[14*i+4] = finalBbox[i].y2;//bottom
        for (int j =0;j<10;j++){
            faceInfo[14*i+5+j]=static_cast<int>(finalBbox[i].ppoint[j]);
        }
    }

    jintArray tFaceInfo = env->NewIntArray(out_size);
    env->SetIntArrayRegion(tFaceInfo,0,out_size,faceInfo);
    //  LOGD("内部人脸检测完成,导出数据成功");
    delete[] faceInfo;
    env->ReleaseByteArrayElements(image_date, imageDate, 0);
    return tFaceInfo;


}


extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_mtcnn_MTCNN_MaxFaceDetect(JNIEnv *env, jobject thiz, jbyteArray image_date,
                                           jint image_width, jint image_height,
                                           jint image_channel) {
    // TODO: implement MaxFaceDetect()
    //  LOGD("JNI开始检测人脸");
    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型SDK未初始化，直接返回空");
        return NULL;
    }

    int tImageDateLen = env->GetArrayLength(image_date);
    if(image_channel == tImageDateLen / image_width / image_height){
        LOGD("数据宽=%d,高=%d,通道=%d",image_width,image_height,image_channel);
    }
    else{
        LOGD("数据长宽高通道不匹配，直接返回空");
        return NULL;
    }

    jbyte *imageDate = env->GetByteArrayElements(image_date, NULL);
    if (NULL == imageDate){
        LOGD("导入数据为空，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    if(image_width<20||image_height<20){
        LOGD("导入数据的宽和高小于20，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    //TODO 通道需测试
    if(3 == image_channel || 4 == image_channel){
        //图像通道数只能是3或4；
    }else{
        LOGD("图像通道数只能是3或4，直接返回空");
        env->ReleaseByteArrayElements(image_date, imageDate, 0);
        return NULL;
    }

    //int32_t minFaceSize=40;
    //mtcnn->SetMinFace(minFaceSize);

    unsigned char *faceImageCharDate = (unsigned char*)imageDate;
    ncnn::Mat ncnn_img;
    if(image_channel==3) {
        ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_BGR2RGB,
                                          image_width, image_height);
    }else{
        ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_RGBA2RGB, image_width, image_height);
    }

    std::vector<Bbox> finalBbox;
    mtcnn->detectMaxFace(ncnn_img, finalBbox);

    int32_t num_face = static_cast<int32_t>(finalBbox.size());
    LOGD("检测到的人脸数目：%d\n", num_face);

    int out_size = 1+num_face*14;
    //  LOGD("内部人脸检测完成,开始导出数据");
    int *faceInfo = new int[out_size];
    faceInfo[0] = num_face;
    for(int i=0;i<num_face;i++){
        faceInfo[14*i+1] = finalBbox[i].x1;//left
        faceInfo[14*i+2] = finalBbox[i].y1;//top
        faceInfo[14*i+3] = finalBbox[i].x2;//right
        faceInfo[14*i+4] = finalBbox[i].y2;//bottom
        for (int j =0;j<10;j++){
            faceInfo[14*i+5+j]=static_cast<int>(finalBbox[i].ppoint[j]);
        }
    }

    jintArray tFaceInfo = env->NewIntArray(out_size);
    env->SetIntArrayRegion(tFaceInfo,0,out_size,faceInfo);
    //  LOGD("内部人脸检测完成,导出数据成功");
    delete[] faceInfo;
    env->ReleaseByteArrayElements(image_date, imageDate, 0);
    return tFaceInfo;



}




extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mtcnn_MTCNN_FaceDetectionModelUnInit(JNIEnv *env, jobject thiz) {
    // TODO: implement FaceDetectionModelUnInit()

    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型已经释放过或者未初始化");
        return true;
    }
    jboolean tDetectionUnInit = false;
    delete mtcnn;


    detection_sdk_init_ok=false;
    tDetectionUnInit = true;
    LOGD("人脸检测初始化锁，重新置零");
    return tDetectionUnInit;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mtcnn_MTCNN_SetMinFaceSize(JNIEnv *env, jobject thiz, jint min_size) {
    // TODO: implement SetMinFaceSize()

    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型SDK未初始化，直接返回");
        return false;
    }

    if(min_size<=20){
        min_size=20;
    }

    mtcnn->SetMinFace(min_size);
    return true;

}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mtcnn_MTCNN_SetThreadsNumber(JNIEnv *env, jobject thiz, jint threads_number) {
    // TODO: implement SetThreadsNumber()
    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型SDK未初始化，直接返回");
        return false;
    }

    if(threads_number!=1&&threads_number!=2&&threads_number!=4&&threads_number!=8){
        LOGD("线程只能设置1，2，4，8");
        return false;
    }

    mtcnn->SetNumThreads(threads_number);
    return  true;

}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mtcnn_MTCNN_SetTimeCount(JNIEnv *env, jobject thiz, jint time_count) {
    // TODO: implement SetTimeCount()

    if(!detection_sdk_init_ok){
        LOGD("人脸检测MTCNN模型SDK未初始化，直接返回");
        return false;
    }

    mtcnn->SetTimeCount(time_count);
    return true;


}


