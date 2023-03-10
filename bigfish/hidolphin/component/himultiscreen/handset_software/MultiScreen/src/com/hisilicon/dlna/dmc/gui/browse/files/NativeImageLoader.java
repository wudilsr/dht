package com.hisilicon.dlna.dmc.gui.browse.files;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import com.hisilicon.dlna.dmc.utility.MyException;
import com.hisilicon.dlna.dmc.utility.ThumbnailGenerator;
import com.hisilicon.dlna.dmc.utility.Utility;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.os.Handler;
import android.os.Message;
import android.support.v4.util.LruCache;
import android.util.Log;

/**
 * 本地图片加载器,采用的是异步解析本地图片，单例模式利用getInstance()获取NativeImageLoader实例
 * 调用loadNativeImage()方法加载本地图片，此类可作为一个加载本地图片的工具类
 * 
 */
public class NativeImageLoader {
	private LruCache<String, Bitmap> mMemoryCache;
	private static NativeImageLoader mInstance = new NativeImageLoader();
	private ExecutorService mImageThreadPool = Executors.newFixedThreadPool(1);
//	private ThreadPoolExecutor threadPool = new ThreadPoolExecutor(1, 6, 3, TimeUnit.SECONDS,new LinkedBlockingQueue<Runnable>());


	private NativeImageLoader(){
		//获取应用程序的最大内存
		final int maxMemory = (int) (Runtime.getRuntime().maxMemory() / 1024);

		//用最大内存的1/4来存储图片
		final int cacheSize = maxMemory / 8;
		System.out.println("The cacheSize is:" + cacheSize);
		mMemoryCache = new LruCache<String, Bitmap>(cacheSize) {

			//获取每张图片的大小
			@Override
			protected int sizeOf(String key, Bitmap bitmap) {
				return bitmap.getRowBytes() * bitmap.getHeight() / 1024;
			}
		};
	}

	/**
	 * 通过此方法来获取NativeImageLoader的实例
	 * @return
	 */
	public static NativeImageLoader getInstance(){
		return mInstance;
	}


	/**
	 * 加载本地图片，对图片不进行裁剪
	 * @param path
	 * @param mCallBack
	 * @return
	 */
	public Bitmap loadNativeImage(final String path, final NativeImageCallBack mCallBack){
		return this.loadNativeImage(path, null, mCallBack);
	}

	/**
	 * 此方法来加载本地图片，这里的mPoint是用来封装ImageView的宽和高，我们会根据ImageView控件的大小来裁剪Bitmap
	 * 如果你不想裁剪图片，调用loadNativeImage(final String path, final NativeImageCallBack mCallBack)来加载
	 * @param path
	 * @param mPoint
	 * @param mCallBack
	 * @return
	 */
	public Bitmap loadNativeImage(final String path, final Point mPoint, final NativeImageCallBack mCallBack){
		//先获取内存中的Bitmap
		Bitmap bitmap = getBitmapFromMemCache(path);
		//		Bitmap bitmap = GlobalCache.getBitmap(path);

		final Handler mHander = new Handler(){

			@Override
			public void handleMessage(Message msg) {
				super.handleMessage(msg);
				mCallBack.onImageLoader((Bitmap)msg.obj, path);
			}

		};

		//若该Bitmap不在内存缓存中，则启用线程去加载本地的图片，并将Bitmap加入到mMemoryCache中
		if(bitmap == null||bitmap.isRecycled()){
			mImageThreadPool.execute(new Thread() {

				@Override
				public void run() {
					//先获取图片的缩略图
					try{
						Bitmap mBitmap = decodeThumbBitmapForFile(path, mPoint == null ? 0: mPoint.x, mPoint == null ? 0: mPoint.y);
						Message msg = mHander.obtainMessage();
						msg.obj = mBitmap;
						mHander.sendMessage(msg);

						//将图片加入到内存缓存
						//						GlobalCache.putToCache(path, mBitmap);
						addBitmapToMemoryCache(path, mBitmap);
					}catch(Exception e){
						e.printStackTrace();
					}

				}
			});
		}
		return bitmap;

	}
	
	/**
	 * 往内存缓存中添加Bitmap
	 * 
	 * @param key
	 * @param bitmap
	 */
	public void addBitmapToMemoryCache(String key, Bitmap bitmap) {
		if (getBitmapFromMemCache(key) == null && bitmap != null) {
			mMemoryCache.put(key, bitmap);
		}
		//		if(mMemoryCache.size()>200){
		//			for(int i=0; i<200; i++){
		//				
		//			}
		//		}
	}


	/**
	 * 根据key来获取内存中的图片
	 * @param key
	 * @return
	 */
	public Bitmap getBitmapFromMemCache(String key) {
		return mMemoryCache.get(key);
	}


	/**
	 * 根据View(主要是ImageView)的宽和高来获取图片的缩略图
	 * @param path
	 * @param viewWidth
	 * @param viewHeight
	 * @return
	 */
	public Bitmap decodeThumbBitmapForFile(String path, int viewWidth, int viewHeight){
		/**
		BitmapFactory.Options options = new BitmapFactory.Options();
		//设置为true,表示解析Bitmap对象，该对象不占内存
		options.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(path, options);
		//设置缩放比例
		options.inSampleSize = computeScale(options, viewWidth, viewHeight);
		if(options.inSampleSize==1){
			options.inSampleSize = 2;
		}
		//		System.out.println("The SampleSize is:" + options.inSampleSize);
		//		options.inSampleSize = 3;

		//设置为false,解析Bitmap对象加入到内存中
		options.inJustDecodeBounds = false;
		try{
			return BitmapFactory.decodeFile(path, options);
		}catch(Exception e){
			e.printStackTrace();
		}catch(Error ex) {
			ex.printStackTrace();
			System.gc();
		}
		
		*/
		try {
			return ThumbnailGenerator.getInstance().decodeFileBitmap(new File(path), viewWidth, viewHeight);
		} catch (MyException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}


	/**
	 * 根据View(主要是ImageView)的宽和高来计算Bitmap缩放比例。默认不缩放
	 * @param options
	 * @param width
	 * @param height
	 */
	private int computeScale(BitmapFactory.Options options, int viewWidth, int viewHeight){
		int inSampleSize = 4;
		if(viewWidth == 0 || viewWidth == 0){
			return inSampleSize;
		}
		int bitmapWidth = options.outWidth;
		int bitmapHeight = options.outHeight;

		//假如Bitmap的宽度或高度大于我们设定图片的View的宽高，则计算缩放比例
		if(bitmapWidth > viewWidth || bitmapHeight > viewWidth){
			int widthScale = Math.round((float) bitmapWidth / (float) viewWidth);
			int heightScale = Math.round((float) bitmapHeight / (float) viewWidth);

			//为了保证图片不缩放变形，我们取宽高比例最小的那个
			inSampleSize = widthScale < heightScale ? widthScale : heightScale;
		}
		return inSampleSize;
	}


	public void clearCache(){
		if(mMemoryCache.size()>0)
			mMemoryCache.trimToSize(mMemoryCache.size());
	}

	/**
	 * 加载本地图片的回调接口
	 * 
	 * @author xiaanming
	 *
	 */
	public interface NativeImageCallBack{
		/**
		 * 当子线程加载完了本地的图片，将Bitmap和图片路径回调在此方法中
		 * @param bitmap
		 * @param path
		 */
		public void onImageLoader(Bitmap bitmap, String path);
	}

}
