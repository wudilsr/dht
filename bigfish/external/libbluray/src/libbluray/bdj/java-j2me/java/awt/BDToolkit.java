/*
 * This file is part of libbluray
 * Copyright (C) 2012  libbluray
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

package java.awt;

import java.awt.image.ColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;
import java.net.URL;
import java.util.Hashtable;
import java.util.Map;
import java.util.Iterator;

import sun.awt.NullGraphics;
import sun.awt.image.ByteArrayImageSource;
import sun.awt.image.FileImageSource;
import sun.awt.image.URLImageSource;

class BDToolkit extends Toolkit {
    private EventQueue eventQueue = new EventQueue();
    private BDGraphicsEnvironment localEnv = new BDGraphicsEnvironment();
    private BDGraphicsConfiguration defaultGC = (BDGraphicsConfiguration)localEnv.getDefaultScreenDevice().getDefaultConfiguration();
    private static Hashtable cachedImages = new Hashtable();
    public BDToolkit () {}

    public static void setFocusedWindow(Window window) {
        /* nothing to do */
    }

    public Dimension getScreenSize() {
        Rectangle dims = defaultGC.getBounds();
        return new Dimension(dims.width, dims.height);
    }

    Graphics getGraphics(Window window) {
        if (!(window instanceof BDRootWindow))
            return new NullGraphics(window);
        return new BDWindowGraphics((BDRootWindow)window);
    }

    GraphicsEnvironment getLocalGraphicsEnvironment() {
        return localEnv;
    }

    public int getScreenResolution() {
        return 72;
    }

    public ColorModel getColorModel() {
        return defaultGC.getColorModel();
    }

    public String[] getFontList() {
        return BDFontMetrics.getFontList();
    }

    public FontMetrics getFontMetrics(Font font) {
        return BDFontMetrics.getFontMetrics(font);
    }

    public void sync() {
        Window window = ((BDGraphicsDevice)localEnv.getDefaultScreenDevice()).getWindow();
        if (window instanceof BDRootWindow)
            ((BDRootWindow)window).sync();
    }

    static void clearCache(BDImage image) {
        synchronized (cachedImages) {
            Iterator i = cachedImages.entrySet().iterator();
            while (i.hasNext()) {
                Map.Entry entry = (Map.Entry) i.next();
                if (entry.getValue() == image) {
                    i.remove();
                    return;
                }
            }
        }
    }

    public Image getImage(String filename) {
        if (cachedImages.containsKey(filename))
            return (Image)cachedImages.get(filename);
        Image newImage = createImage(filename);
        if (newImage != null)
            cachedImages.put(filename, newImage);
        return newImage;
    }

    public Image getImage(URL url) {
        if (cachedImages.containsKey(url))
            return (Image)cachedImages.get(url);
        Image newImage = createImage(url);
        if (newImage != null)
            cachedImages.put(url, newImage);
        return newImage;
    }

    public Image createImage(String filename) {
        ImageProducer ip = new FileImageSource(filename);
        Image newImage = createImage(ip);
        return newImage;
    }

    public Image createImage(URL url) {
        ImageProducer ip = new URLImageSource(url);
        Image newImage = createImage(ip);
        return newImage;
    }

    public Image createImage(byte[] imagedata,
        int imageoffset,
        int imagelength) {
        ImageProducer ip = new ByteArrayImageSource(imagedata, imageoffset, imagelength);
        Image newImage = createImage(ip);
        return newImage;
    }

    public Image createImage(ImageProducer producer) {
        return new BDImageConsumer(producer);
    }

    public Image createImage(Component component, int width, int height) {
        return new BDImage(component, width, height, defaultGC);
    }

    public boolean prepareImage(Image image, int width, int height, ImageObserver observer) {
        if (!(image instanceof BDImageConsumer))
            return true;
        BDImageConsumer img = (BDImageConsumer)image;
        return img.prepareImage(observer);
    }

    public int checkImage(Image image, int width, int height,
        ImageObserver observer) {
        if (!(image instanceof BDImageConsumer)) {
            return ImageObserver.ALLBITS;
        }
        BDImageConsumer img = (BDImageConsumer)image;
        return img.checkImage(observer);
    }

    public void beep() { }

    protected EventQueue getSystemEventQueueImpl() {
        return eventQueue;
    }
}
