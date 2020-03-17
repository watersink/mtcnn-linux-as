package com.example.mtcnn;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

import static android.content.ContentValues.TAG;

public class MainActivity extends Activity {

    private static final int SELECT_IMAGE = 1;

    private TextView infoResult;
    private ImageView imageView;
    private Bitmap yourSelectedImage = null;


    private int minFaceSize = 5;
    private int testTimeCount = 1;
    private int threadsNumber = 2;

    private boolean maxFaceSetting = false;


    private MTCNN mtcnn = new MTCNN();



    private void init_mtcnn() throws IOException {
        byte[] det1_param = null;
        byte[] det1_bin = null;
        byte[] det2_param = null;
        byte[] det2_bin = null;
        byte[] det3_param = null;
        byte[] det3_bin = null;

        //det1
        {
            //用io流读取二进制文件，最后存入到byte[]数组中
            InputStream assetsInputStream = getAssets().open("det1.param.bin");// param：  网络结构文件
            int available = assetsInputStream.available();
            det1_param = new byte[available];
            int byteCode = assetsInputStream.read(det1_param);
            assetsInputStream.close();
        }
        {
            //用io流读取二进制文件，最后存入到byte上，转换为int型
            InputStream assetsInputStream = getAssets().open("det1.bin");//bin：   model文件
            int available = assetsInputStream.available();
            det1_bin = new byte[available];
            int byteCode = assetsInputStream.read(det1_bin);
            assetsInputStream.close();
        }
        //det2
        {
            //用io流读取二进制文件，最后存入到byte[]数组中
            InputStream assetsInputStream = getAssets().open("det2.param.bin");// param：  网络结构文件
            int available = assetsInputStream.available();
            det2_param = new byte[available];
            int byteCode = assetsInputStream.read(det2_param);
            assetsInputStream.close();
        }
        {
            //用io流读取二进制文件，最后存入到byte上，转换为int型
            InputStream assetsInputStream = getAssets().open("det2.bin");//bin：   model文件
            int available = assetsInputStream.available();
            det2_bin = new byte[available];
            int byteCode = assetsInputStream.read(det2_bin);
            assetsInputStream.close();
        }

        //det3
        {
            //用io流读取二进制文件，最后存入到byte[]数组中
            InputStream assetsInputStream = getAssets().open("det3.param.bin");// param：  网络结构文件
            int available = assetsInputStream.available();
            det3_param = new byte[available];
            int byteCode = assetsInputStream.read(det3_param);
            assetsInputStream.close();
        }
        {
            //用io流读取二进制文件，最后存入到byte上，转换为int型
            InputStream assetsInputStream = getAssets().open("det3.bin");//bin：   model文件
            int available = assetsInputStream.available();
            det3_bin = new byte[available];
            int byteCode = assetsInputStream.read(det3_bin);
            assetsInputStream.close();
        }


        mtcnn.FaceDetectionModelInit(det1_param,det1_bin,det2_param,det2_bin,det3_param,det3_bin);
    }




    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);



        try
        {
            init_mtcnn();
        } catch (IOException e) {
            Log.e("MainActivity", "initmtcnn error");
        }

        Log.i(TAG, "初始化完成：");


        infoResult = (TextView) findViewById(R.id.infoResult);
        imageView = (ImageView) findViewById(R.id.imageView);



        Button buttonImage = (Button) findViewById(R.id.buttonImage);
        buttonImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                yourSelectedImage = BitmapFactory.decodeResource(getResources(), R.drawable.beauty);
                imageView.setImageBitmap(yourSelectedImage);
            }
        });

        Button buttonDetect = (Button) findViewById(R.id.buttonDetect);
        buttonDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (yourSelectedImage == null)
                    return;


                mtcnn.SetMinFaceSize(minFaceSize);
                mtcnn.SetTimeCount(testTimeCount);
                mtcnn.SetThreadsNumber(threadsNumber);

                //检测流程
                int width = yourSelectedImage.getWidth();
                int height = yourSelectedImage.getHeight();
                byte[] imageDate = getPixelsRGBA(yourSelectedImage);

                long timeDetectFace = System.currentTimeMillis();
                int faceInfo[] = null;
                if(!maxFaceSetting) {
                    faceInfo = mtcnn.FaceDetect(imageDate, width, height, 4);
                    Log.i(TAG, "检测所有人脸");
                }
                else{
                    faceInfo = mtcnn.MaxFaceDetect(imageDate, width, height, 4);
                    Log.i(TAG, "检测最大人脸");
                }
                timeDetectFace = System.currentTimeMillis() - timeDetectFace;
                Log.i(TAG, "人脸平均检测时间："+timeDetectFace/testTimeCount);

                if(faceInfo.length>1){
                    int faceNum = faceInfo[0];
                    infoResult.setText("图宽："+width+"高："+height+"人脸平均检测时间："+timeDetectFace/testTimeCount+" 数目：" + faceNum);
                    Log.i(TAG, "图宽："+width+"高："+height+" 人脸数目：" + faceNum );

                    Bitmap drawBitmap = yourSelectedImage.copy(Bitmap.Config.ARGB_8888, true);
                    for(int i=0;i<faceNum;i++) {
                        int left, top, right, bottom;
                        Canvas canvas = new Canvas(drawBitmap);
                        Paint paint = new Paint();
                        left = faceInfo[1+14*i];
                        top = faceInfo[2+14*i];
                        right = faceInfo[3+14*i];
                        bottom = faceInfo[4+14*i];
                        paint.setColor(Color.RED);
                        paint.setStyle(Paint.Style.STROKE);//不填充
                        paint.setStrokeWidth(15);  //线的宽度
                        canvas.drawRect(left, top, right, bottom, paint);
                        //画特征点
                        canvas.drawPoints(new float[]{faceInfo[5+14*i],faceInfo[10+14*i],
                                faceInfo[6+14*i],faceInfo[11+14*i],
                                faceInfo[7+14*i],faceInfo[12+14*i],
                                faceInfo[8+14*i],faceInfo[13+14*i],
                                faceInfo[9+14*i],faceInfo[14+14*i]}, paint);//画多个点
                    }
                    imageView.setImageBitmap(drawBitmap);
                }else{
                    infoResult.setText("未检测到人脸");
                }

            }
        });



    }


    //提取像素点
    private byte[] getPixelsRGBA(Bitmap image) {
        // calculate how many bytes our image consists of
        int bytes = image.getByteCount();
        ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
        image.copyPixelsToBuffer(buffer); // Move the byte data to the buffer
        byte[] temp = buffer.array(); // Get the underlying array containing the

        return temp;
    }

}
