package com.hisilicon.dlna.dmc.gui.browse.files;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.ImageView;

public class MyImageView extends ImageView{

	public MyImageView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		// TODO Auto-generated constructor stub
	}
	
	
	public MyImageView(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}



	public MyImageView(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
	}



	@Override
	protected void onDraw(Canvas canvas) {
		// TODO Auto-generated method stub
		try {  
			super.onDraw(canvas);  
		} catch (Exception e) {  
			System.out  
			.println("MyImageView  -> onDraw() Canvas: trying to use a recycled bitmap");  
		}  

	}
	
	

}
