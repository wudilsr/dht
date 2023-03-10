package com.hisilicon.multiscreen.server;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;

import org.apache.http.util.EncodingUtils;
import org.json.JSONObject;
import org.xml.sax.SAXException;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.content.res.Resources.Theme;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.net.ethernet.EthernetManager;
import android.provider.Settings;
import android.provider.Settings.Global;
import android.media.AudioManager;

import com.hisilicon.multiscreen.protocol.message.Action;
import com.hisilicon.multiscreen.protocol.message.Argument;
import com.hisilicon.multiscreen.protocol.message.ArgumentValue;
import com.hisilicon.multiscreen.protocol.message.DefaultResponseMessage;
import com.hisilicon.multiscreen.protocol.message.PlayMediaMessage;
import com.hisilicon.multiscreen.protocol.message.PushMessage;
import com.hisilicon.multiscreen.protocol.message.PushMessageHead;
import com.hisilicon.multiscreen.protocol.scene.IServerSceneListener;
import com.hisilicon.multiscreen.protocol.scene.SceneObserver;
import com.hisilicon.multiscreen.protocol.scene.SceneType;
import com.hisilicon.multiscreen.protocol.server.IPushMessageHandler;
import com.hisilicon.multiscreen.protocol.server.PushServer;
import com.hisilicon.multiscreen.protocol.utils.AppListTransmitter;
import com.hisilicon.multiscreen.protocol.utils.LogTool;
import com.hisilicon.multiscreen.protocol.utils.MultiScreenIntentAction;
import com.hisilicon.multiscreen.protocol.utils.PronounceCoverter;
import com.hisilicon.multiscreen.protocol.utils.SaxXmlUtil;
import com.hisilicon.multiscreen.server.speech.DialogUtil;
import com.hisilicon.multiscreen.server.speech.SpeechDialog;
import com.hisilicon.multiscreen.server.speech.responsemessage.SpeechResponseMessage;

/**
 * MultiScreen service class.<br>
 * CN:??????????????????
 */
public class MultiScreenService extends Service implements IPushMessageHandler
{

    /**
     * Intent Action, DLNA PlayerBroadcastReceiver.<br>
     * CN:??????DLNA????????????Intent???
     */
    public static final String BROADCAST_ACTION_DLNA_PLAYER =
        "com.hisilicon.dlna.dmr.action.player";

    /**
     * Path of 3D mode notice file.<br>
     * CN:3D?????????????????????????????????
     */
    private static final String NOTICE_3D_MODE_FILE_PATH =
        "data/data/com.hisilicon.multiscreen.server/cache/mirror_3d_mode.jpg";

    /**
     * Path of h264 compatible device list.<br>
     * CN: h264????????????????????????????????????
     */
    private static final String COMPATIABLE_H264_LIST_FILE_PATH =
        "data/data/com.hisilicon.multiscreen.server/cache/compath264list.xml";

    /**
     * Notify enter smart suspend.<br>
     * CN: ????????????????????????
     */
    private static final String SMART_SUSPEND_ENTER = "smart_suspend_broadcast_enter";

    /**
     * Notify quit smart suspend.<br>
     * CN: ????????????????????????
     */
    private static final String SMART_SUSPEND_QUIT = "smart_suspend_broadcast_quit";

    /**
     * Wifi net interface card name.<br>
     */
    private static final String WIFI_NIC_NAME = "wlan0";

    /**
     * Default Net interface card name.<br>
     */
    private static final String NIC_DEFAULT_NAME = "unknow";

    private final static String DEFAULT_USE_ADAPTER_WIFI_ETHERNET = "eth";

    /**
     * Message id of start all server.<br>
     * CN:???????????????????????????ID???
     */
    private static final int MSG_RESTART_MULTISCREEN = 0x100;

    /**
     * Message id of stop all server.<br>
     * CN:???????????????????????????ID???
     */
    private static final int MSG_STOP_MULTISCREEN = 0x101;

    /**
     * Message id of start speech.<br>
     * CN:?????????????????????ID???
     */
    private static final int MSG_START_SPEECH = 0x102;

    /**
     * Time of delay handling message.<br>
     * CN:?????????????????????
     */
    private final long DELAY_MILLIS_HANDLE = 300;

    /**
     * Time of delay stopping services to notify client if STB is manual off.<br>
     * CN:????????????STB??????????????????????????????????????????????????????????????????
     */
    private final long DELAY_MILLIS_MANUAL_OFF = 2000;

    /**
     * Default sender id.<br>
     * CN:????????????????????????ID???
     */
    private static final int SENDER_DEFAULT = 0;

    /**
     * Sender id of network change.<br>
     * CN:??????????????????????????????ID???
     */
    private static final int SENDER_NETWORK_CHANGE = SENDER_DEFAULT + 1;

    /**
     * Sender id of manual switch.<br>
     * CN:??????????????????????????????ID???
     */
    private static final int SENDER_MANUAL_SWITCH = SENDER_DEFAULT + 2;

    /**
     * Remote app's server port for receive client's data.<br>
     * CN:???????????????????????????
     */
    private final int PUSHSERVER_SERVICE_PORT = 8867;

    /**
     * Max buffer size for allocate byte[].<br>
     * CN:byte ???????????????size???
     */
    private final int MAX_BYTE_BUFFER_SIZE = 2048;

    /**
     * Class for jni call.<br>
     * CN:??????jni???????????????
     */
    private static MultiScreenNative mMultiScreenNative = null;

    /**
     * Service's context.<br>
     * CN:?????????????????????
     */
    private Context mContext = null;

    /**
     * Flag for VIME control server start or stop.<br>
     * CN:remote app ???????????????????????????
     */
    private boolean mIsVIMEControlServerStart = false;

    /**
     * Flag for remote app start or stop.<br>
     * CN:remote app ???????????????????????????
     */
    private boolean mIsRemoteAppServerStart = false;

    /**
     * Switch state manager.<br>
     * CN:HiMultiScreen????????????????????????
     */
    private SwitchStateManager stateManager = null;

    /**
     * Register or unregister mNetworkChangeReceiver when switch changes.<br>
     * CN:???????????????????????????????????????????????????
     */
    private BroadcastReceiver mSwitchChangeReceiver = null;

    /**
     * Restart or stop discovery server when network status changes.<br>
     * CN:???????????????????????????????????????????????????
     */
    private BroadcastReceiver mNetworkChangeReceiver = null;

    private BroadcastReceiver mPowerReceiver = null;

    /**
     * Is upnp stack open.<br>
     * CN:upnp??????????????????
     */
    private static boolean isUpnpStackOpen = false;

    /**
     * Handler of message.<br>
     * CN:??????????????????
     */
    private MyHandler myHandler = null;

    /**
     * Thread of handler.<br>
     * CN:?????????????????????
     */
    private HandlerThread mHandlerThread = null;

    /**
     * Enum of network status.<br>
     * CN:?????????????????????
     */
    enum NetStatus
    {
        NetworkOn, NetworkOff
    }

    /**
     * Active net interface name.<br>
     * CN:????????????????????????
     */
    private String mActiveNetType = NIC_DEFAULT_NAME;

    /**
     * Status of network.<br>
     * CN:???????????????
     */
    private static NetStatus netOldState = NetStatus.NetworkOff;

    /**
     * PushServer.<br>
     * CN:PushServer?????????
     */
    private PushServer mPushServer = null;

    /**
     * Previous app package name.<br>
     * CN:?????????app????????????
     */
    private String mPrevAppPackageName = null;

    private String hostName = null;

    /**
     * VIme control class.<br>
     * CN:VIme ????????????
     */
    private VImeControl mVImeControl = null;

    /**
     * Scene observer.<br>
     * CN:?????????????????????
     */
    private SceneObserver mSceneObserver = null;

    /**
     * xml???????????????.<br>
     * CN:xml??????????????????
     */
    private SaxXmlUtil mSaxXmlTool = null;

    /**
     * the helper class for speech. CN:??????????????????
     */
    // private SpeechRecognitionHelper mRecognitionHelper = null;

    /**
     * speech Dialog.<br>
     * CN: ??????dialog.
     */
    private SpeechDialog mSpeechDialog = null;

    /**
     * CST flag.<br>
     * CN: CTS?????????.
     */
    private String ctsEnable = null;

    /**
     * Callback class.<br>
     * CN:?????????.<br>
     */
    private MultisScreenNativeCallback mNativeCallback = new MultisScreenNativeCallback()
    {
        @Override
        public int nativeInvoke(int comandId, String param, String reservePara)
        {
            int result = MultiScreenNativeCommand.COMMAND_RET_FAIL;
            MultiScreenNativeCommand.NativeInvokeCommand command =
                MultiScreenNativeCommand.NativeInvokeCommand.getCommand(comandId);
            if (null == command)
            {
                LogTool.e("command is null");
                return result;
            }
            switch (command)
            {
                case START_VIME_CONTROL:
                {
                    result = startVIMEControl();
                    break;
                }

                case STOP_VIME_CONTROL:
                {
                    result = stopVIMEControl();
                    break;
                }

                case SET_VIME_URL:
                {
                    result = setVIMEClientURL(param);
                    break;
                }

                case START_REMOTE_APP:
                {
                    result = startRemoteApp();
                    break;
                }

                case STOP_REMOTE_APP:
                {
                    result = stopRemoteApp();
                    break;
                }
                case NOTIFY_SUSPEND_QUIT:
                {
                    result = notifySuspendQuit();
                    break;
                }
                default:
                {
                    LogTool.e("native invoke invalide command");
                    result = MultiScreenNativeCommand.COMMAND_RET_FAIL;
                    break;
                }
            }
            LogTool.i("call command result " + result);
            return result;
        }
    };

    /**
     * Speech msg deal helper.<br>
     * CN:??????????????????????????????<br>
     */
    private ISpeechMsgDealHelper mSpeechMsgDealHelper = new ISpeechMsgDealHelper()
    {

        @Override
        public void speechMsgDeal(String msg)
        {
            LogTool.i("");
            String keyword = PronounceCoverter.getInstance().getPingYin(msg);
            String Data = "page=search&keyword=" + keyword;
            LogTool.i("keyword " + Data);
            String ReturnMode = "0";
            Intent intent = new Intent();
            intent.setAction("moretv.action.applaunch"); // action ?????? ??????
            intent.putExtra("Data", Data); // Date ?????????????????????
            intent.putExtra("ReturnMode", ReturnMode); // ReturnMode ???????????? 0:
                                                       // ???????????????MoreTV?????? 1:
                                                       // ???????????????MoreTV???????????? ???????????????
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    };

    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        LogTool.d("onCreate.");

        mContext = this;
        ctsEnable = SystemProperties.get("persist.sys.cts.enable");
        if(null == ctsEnable)
        {
            ctsEnable = "false";
        }

        if(ctsEnable.equals("true"))
        {
            LogTool.d("do nothing in service");
            return ;
        }
        initModules();
        registerReceivers();
        setServiceForeground();
    }

    /**
     * Stop services on destroy.<br>
     * CN:????????????????????????
     */
    public void onDestroy()
    {
        super.onDestroy();
        LogTool.d("onDestroy.");
        if(ctsEnable.equals("true"))
        {
            LogTool.d("do nothing in service");
            return ;
        }
        else
        {
            unregisterReceivers();
            deinitModules();
            stopUpnpStack();
        }
    }

    /**
     * Init modules.<br>
     * CN:??????????????????
     */
    private void initModules()
    {
        initSwitchStateManager();

        // Push server service. CN:?????????????????????
        initPushServer();

        // Virtual IME control server. CN:??????????????????????????????
        initVIMEControlServer();

        // Init mirror 3D mode notice file. CN:???????????????3D?????????????????????
        initMirror3DModeNoticeFile();

        // Init compatable list file
        initMirrorCompatListFile();

        // Initialize MultiScreen Native.
        initMultiScreenNative();

        // Initialize network change handler.
        initMessageHandler();

        initSpeechModule();

        // close scene because no one use it
        //initSceneObserver();
    }

    private void deinitModules()
    {
        // close scene because no one use it
        //deinitSceneObserver();
        deinitMessageHandler();
        deinitSwitchStateManager();
    }

    /**
     * Register receivers.<br>
     * Invoke it after initMultiScreenNative.<br>
     * CN:????????????????????????<br>
     * CN:?????????initMultiScreenNative????????????
     */
    private void registerReceivers()
    {
        // CN:???????????????
        registerSwitchReceiver();

        // CN:????????????????????????????????????????????????????????????????????????
        if (stateManager.isOpen())
        {
            registerNetworkBroadcastReceiver();
            registerPowerBroadcastReceiver();
        }
        else
        {
            LogTool.d("Switch is closed when MultiScreenService on create.");
        }
    }

    /**
     * Unregister receivers.<br>
     * CN:???????????????????????????
     */
    private void unregisterReceivers()
    {
        unregisterSwitchReceiver();
        unregisterNetworkBroadcastReceiver();
        unregisterPowerReceiver();
    }

    private void initSwitchStateManager()
    {
        stateManager = SwitchStateManager.getInstance(this);
        stateManager.init();
    }

    private void deinitSwitchStateManager()
    {
        stateManager.deinit();
    }

    private void initMessageHandler()
    {
        mHandlerThread = new HandlerThread("multiscreen_service_handlerthread");
        mHandlerThread.start();
        myHandler = new MyHandler(mHandlerThread.getLooper());
    }

    private void deinitMessageHandler()
    {
        if (myHandler != null)
        {
            clearMessages();
            myHandler = null;
        }

        if (mHandlerThread != null)
        {
            mHandlerThread.getLooper().quit();
        }
    }

    private void initSpeechModule()
    {

    }

    /**
     * FIXME CN:???????????????
     */
    private void initSceneObserver()
    {
        // CN:???????????????UI???????????????
        IServerSceneListener listener = new IServerSceneListener()
        {
            @Override
            public void notifyScene(String type)
            {
                mMultiScreenNative.native_MultiScreenNotifySceneRecognition(type);
            }
        };

        if (mSceneObserver == null)
        {
            mSceneObserver = new SceneObserver(mContext, listener);
        }
        else
        {
            mSceneObserver.setParameters(mContext, listener);
        }
        mSceneObserver.start();
    }

    private void deinitSceneObserver()
    {
        mSceneObserver.stop();
    }

    private String readFriendlyName()
    {
        // // Put the switch in system setting.
        // SharedPreferences prefrence =
        // getSharedPreferences(MultiScreenServerActivity.MULTISCREEN_SETTING_FILE_NAME,
        // Context.MODE_PRIVATE);
        // return
        // prefrence.getString(MultiScreenServerActivity.MULTISCREEN_FRIENDLY_KEY_NAME,
        // MultiScreenServerActivity.DEFAULT_FRIENDLY_NAME);

        SharedPreferences prefrence =
                        getSharedPreferences("MultiScreenSetting",
                            Context.MODE_WORLD_READABLE|Context.MODE_MULTI_PROCESS);
        String devicename = prefrence.getString(MultiScreenServerActivity.MULTISCREEN_FRIENDLY_KEY_NAME,
            Settings.Global.getString(getContentResolver(), getSystemSettingDeviceNameKey()));

        if (null == devicename)
        {
            devicename = getString(R.string.default_device_name);
        }

        return devicename;
    }

    private String getSystemSettingDeviceNameKey()
    {
        String key = "device_name";
        try
        {
            Field deviceNameField = Settings.Global.class.getField("DEVICE_NAME");
            key = (String) deviceNameField.get(null);
        }
        catch (NoSuchFieldException e)
        {
            LogTool.e(e.getMessage());
        }
        catch (SecurityException e)
        {
            LogTool.e(e.getMessage());
        }
        catch (IllegalArgumentException e)
        {
            LogTool.e(e.getMessage());
        }
        catch (IllegalAccessException e)
        {
            LogTool.e(e.getMessage());
        }
        return key;
    }

    /**
     * Set service foreground.<br>
     * CN:?????????????????????????????????
     */
    private void setServiceForeground()
    {
        Notification notification =
            new Notification(R.drawable.muap_server, getText(R.string.himultiscreen_is_running),
                System.currentTimeMillis());
        notification.setLatestEventInfo(this, getText(R.string.FriendlyName),
            getText(R.string.himultiscreen_is_running), null);
        startForeground(R.string.himultiscreen_is_running, notification);
    }

    @Override
    public PushMessage handle(PushMessage msg)
    {
        int type = msg.getHead().getType();
        PushMessage responseMessage;
        switch (type)
        {
            case PushMessageHead.PLAY_MEDIA:
            {
                PlayMediaMessage playMediaReq = (PlayMediaMessage) msg;
                int mediaType = playMediaReq.getMediaType();
                String url = playMediaReq.getUrl();
                LogTool.v("RECV_MEDIA_URL: " + Integer.toString(mediaType) + "/" + url);
                responseMessage = playMedia(playMediaReq);
                break;
            }
            default:
            {
                DefaultResponseMessage res = new DefaultResponseMessage();
                res.setCode(0);
                responseMessage = res;
                break;
            }
        }

        return responseMessage;
    }

    @Override
    public String handleByteArray(byte[] msg) throws ParserConfigurationException, SAXException,
        IOException
    {
        Action responseAction = null;
        /*
         * Analysis httpReq
         */
        Action receiveAction = mSaxXmlTool.parse(msg);

        if (receiveAction != null)
        {
            switch (receiveAction.getId())
            {
            /* Get application list. */
                case Action.ACTION_ID_REMOTE_APP_UPDATE_LIST:
                {
                    responseAction = new Action();
                    responseAction.setId(Action.ACTION_ID_REMOTE_APP_RET_UPDATE_LIST);
                    responseAction.setName("retGetAppList");
                    responseAction =
                        AppListTransmitter.toTransmitPacket(responseAction,
                            this.getPackageManager());
                    if (responseAction != null)
                    {
                        responseAction.setResponseFlag("no");
                        responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
                    }
                    break;
                }

                /* Launch application. */
                case Action.ACTION_ID_REMOTE_APP_LAUNCH:
                {
                    Argument recvArg = receiveAction.getArgument(0);
                    if (null == recvArg)
                    {
                        LogTool.e("app launch action's argument is null");
                        break;
                    }

                    ArgumentValue recvArgVal = recvArg.getArgumentValue(0);
                    if (null == recvArgVal)
                    {
                        LogTool.e("app launch action's argument value is null");
                        break;
                    }
                    Object pkgName = recvArgVal.getVaule();
                    if (pkgName == null)
                    {
                        LogTool.e("app launch action's argument value object is null.");
                        break;
                    }
                    ApplicationRunThread t = new ApplicationRunThread(this, pkgName.toString());
                    t.setName("ApplicationRunThread");
                    t.setDaemon(true);
                    t.start();

                    responseAction = new Action();
                    responseAction.setId(Action.ACTION_ID_REMOTE_APP_RET_LAUNCH);
                    responseAction.setName("retLaunchAPP");
                    responseAction.setResponseFlag("no");
                    responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
                    break;
                }

                case Action.ACTION_ID_SPEECH_TEXT_SEND:
                {
                    LogTool.d("receive speech text");
                    Argument recvArg = receiveAction.getArgument(0);
                    if (null == recvArg)
                    {
                        LogTool.e("send speech info action's argument is null");
                        break;
                    }

                    ArgumentValue recvArgVal = recvArg.getArgumentValue(0);
                    if (null == recvArgVal)
                    {
                        LogTool.e("send speech info action's argument value is null");
                        break;
                    }

                    ArgumentValue recvArgType = recvArg.getArgumentValue(1);
                    if (null == recvArgType)
                    {
                        LogTool.e("send speech info action's argument value is null");
                        break;
                    }

                    int resultType = (Integer) recvArgType.getVaule();
                    LogTool.i("received speech result type " + resultType);
                    String resultJson = (String) recvArgVal.getVaule();
                    LogTool.i("received orignal json result " + resultJson);

                    if ((mSpeechDialog != null) && (mSpeechDialog.isShowing()))
                    {
                        mSpeechDialog.dealMscRespMsg(resultJson, resultType);
                    }

                    responseAction = new Action();
                    responseAction.setId(Action.ACTION_ID_SPEECH_TEXT_RET_SEND);
                    responseAction.setName("retSendSpeechInfo");
                    responseAction.setResponseFlag("no");
                    responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
                    break;
                }

                case Action.ACTION_ID_START_SPEAKING:
                {
                    myHandler.sendEmptyMessage(MSG_START_SPEECH);
                    LogTool.d("start speaking");
                    responseAction = new Action();
                    responseAction.setId(Action.ACTION_ID_RET_START_SPEAKING);
                    responseAction.setName("retStartSpeaking");
                    responseAction.setResponseFlag("no");
                    responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
                    break;
                }

                case Action.ACTION_ID_SPEECH_ERROR:
                {
                    myHandler.sendEmptyMessage(SpeechDialog.MSG_SPEECH_ERROR);
                    LogTool.d("speech error");
                    responseAction = new Action();
                    responseAction.setId(Action.ACTION_ID_SPEECH_ERROR_RET_SEND);
                    responseAction.setName("retSpeechError");
                    responseAction.setResponseFlag("no");
                    responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
                    break;
                }
                default:
                {
                    /* id of action received is not defined */
                    LogTool.e("Request action id is error, so it will send default response!");
                    break;
                }
            }
        }

        if (responseAction == null)
        {
            // default content of response is null.
            responseAction = new Action();
            responseAction.setId(Action.ACTION_ID_NO_RESPONSE);
            responseAction.setName("defaultResponse");
            responseAction.setResponseFlag("no");
            responseAction.setResponseId(Action.ACTION_ID_NO_RESPONSE);
        }

        String responseString;
        try
        {
            responseString = mSaxXmlTool.serialize(responseAction);
        }
        catch (TransformerConfigurationException e)
        {
            responseString = null;
            LogTool.e(e.getMessage());
        }

        return responseString;
    }

    /**
     * Get upnp stack state.<br>
     * CN:UPNP????????????????????????
     * @return true if opened.
     */
    public static boolean isStackOpen()
    {
        return isUpnpStackOpen;
    }

    /**
     * Init file of 3D mode notice picture on STB.<br>
     * CN:???STB????????????3D???????????????????????????
     */
    private void initMirror3DModeNoticeFile()
    {
        File file = new File(NOTICE_3D_MODE_FILE_PATH);
        // file = new File(Environment.getExternalStorageDirectory().getPath() +
        // "/mirror_3dmode.jpg");

        // CN:???????????????
        Resources res = getResources();
        Bitmap bmp = BitmapFactory.decodeResource(res, R.drawable.mirror_3d_mode);
        if (null == bmp)
        {
            LogTool.e("decode resource mirror_3d_mode failed");
            return;
        }
        // CN:??????JPEG
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bmp.compress(Bitmap.CompressFormat.JPEG, 100, baos);
        // CN:???????????????
        byte[] data = baos.toByteArray();
        if (false == writeFile(file.getPath(), data))
        {
            LogTool.e("Write file of 3D mode notice fail!");
        }

        // recycle.
        bmp.recycle();
        try
        {
            baos.close();
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }
    }

    /**
     * Init Mirror compat list file.<br>
     * CN:?????????Mirror??????????????????
     */
    private void initMirrorCompatListFile()
    {
        // CN:???????????????
        Resources res = getResources();
        InputStream istream = res.openRawResource(R.raw.compath264list);
        byte[] buffer = new byte[MAX_BYTE_BUFFER_SIZE];
        int readLen;
        try
        {
            readLen = istream.read(buffer);
        }
        catch (IOException e)
        {
            readLen = -1;
            LogTool.e(e.getMessage());
        }
        try
        {
            istream.close();
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }

        if (-1 != readLen)
        {
            String compatFile = EncodingUtils.getString(buffer, "UTF-8");
            File xmlFile = new File(COMPATIABLE_H264_LIST_FILE_PATH);
            if (xmlFile.exists())
            {
                LogTool.i("file is already exist not override files");
                return;
            }
            writeFile(COMPATIABLE_H264_LIST_FILE_PATH, compatFile);
        }
        else
        {
            LogTool.e("read compatlist file failed");
        }
    }

    private void writeFile(String fileName, String writestr)
    {
        FileOutputStream fout;
        try
        {
            fout = new FileOutputStream(fileName);
        }
        catch (FileNotFoundException e)
        {
            LogTool.e(e.getMessage());
            return;
        }

        byte[] bytes = writestr.getBytes();
        try
        {
            fout.write(bytes);
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }
        try
        {
            fout.close();
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }
    }

    /**
     * Write file on STB.<br>
     * CN:???????????????
     * @param fileName - filepath
     * @param data - date to write
     * @return - true if success.
     */
    private boolean writeFile(String fileName, byte[] data)
    {
        FileOutputStream fout;
        try
        {
            fout = new FileOutputStream(fileName);
        }
        catch (FileNotFoundException e1)
        {
            LogTool.e(e1.getMessage());
            return false;
        }
        try
        {
            fout.write(data);
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }

        try
        {
            fout.close();
        }
        catch (IOException e)
        {
            LogTool.e(e.getMessage());
        }
        return true;
    }

    /**
     * Play media of pushing URL.<br>
     * CN:??????????????????????????????
     * @param msg - media message.<br>
     *        CN:?????????????????????
     */
    private PushMessage playMedia(PlayMediaMessage msg)
    {
        Uri uri = null;
        int type = msg.getMediaType();
        ImageUrlSave saveImg = new ImageUrlSave();
        /*
         * Get MIME of URL. image will not be opened if exact type is used.
         * CN:??????URL???MIME???FIXME ??????????????????MIME???????????????TIF???????????????????????????????????????????????????
         */
        String MIME = saveImg.getMIME(msg.getUrl());
        LogTool.v("MIME = " + MIME);
        // Check URL.CN:URL??????????????????
        if (MIME != null)
        {
            switch (type)
            {
                case PlayMediaMessage.MEDIA_TYPE_VEDIO:
                    MIME = "video/*";
                    uri = Uri.parse(msg.getUrl());
                    break;

                case PlayMediaMessage.MEDIA_TYPE_AUDIO:
                    MIME = "audio/*";
                    uri = Uri.parse(msg.getUrl());
                    break;

                case PlayMediaMessage.MEDIA_TYPE_IMAGE:
                    MIME = "image/*";
                    /*
                     * Down load the image file first and return URI. it has
                     * been replaced by DLNA player!
                     * CN??????????????????URL????????????????????????URI???????????????DLNA?????????
                     */
                    uri = saveImg.getImagePath(msg.getUrl());
                    break;

                default:
                    break;
            }

            if (uri != null)
            {
                Intent i = new Intent();
                i.setAction(Intent.ACTION_VIEW);
                i.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                i.setDataAndType(uri, MIME);
                LogTool.v("start activity to open url");
                startActivity(i);
            }
        }
        else
        {
            LogTool.e("URL received is not correct!");
        }

        // respond client.CN:???????????????
        DefaultResponseMessage res = new DefaultResponseMessage();
        PushMessageHead head = new PushMessageHead();
        head.setType(PushMessageHead.DEFAULT_RESPONSE);
        res.setHead(head);
        res.setCode(0); // 0 means no error
        return res;
    }

    /**
     * Application running control thread.<br>
     * CN:?????????????????????????????????
     */
    private class ApplicationRunThread extends Thread
    {
        private Context mContext;

        private String mPackageName;

        public ApplicationRunThread(Context context, String pkgName)
        {
            this.mContext = context;
            this.mPackageName = pkgName;
        }

        public void run()
        {
            try
            {
                if (mPackageName == null || (mPackageName.trim().length() == 0))
                {
                    return;
                }

                /*
                 * List all applications, and find out by package name.<br>
                 * CN:???????????????????????????????????????????????????????????????
                 */
                Intent i = new Intent(Intent.ACTION_MAIN, null);
                i.addCategory(Intent.CATEGORY_LAUNCHER);
                List<ResolveInfo> mAppsList = getPackageManager().queryIntentActivities(i, 0);

                if (mAppsList == null)
                {
                    mAppsList = new ArrayList<ResolveInfo>();
                }

                String pkgClassName = null;
                for (Iterator<ResolveInfo> it = mAppsList.iterator(); it.hasNext();)
                {
                    ResolveInfo app = it.next();
                    if (app.activityInfo.packageName.equalsIgnoreCase(mPackageName))
                    {
                        pkgClassName = app.activityInfo.name;
                        break;
                    }
                }

                if (null == pkgClassName)
                {
                    LogTool.e("Not found class name by package name:" + mPackageName);
                    return;
                }

                LogTool.v("Package:" + mPackageName + " / Class:" + pkgClassName);

                // Now Kill Previous APP.<br> CN:????????????????????????
                if ((mPrevAppPackageName != null) && (mPrevAppPackageName.length() > 0))
                {
                    if (mPrevAppPackageName.equals("com.hisilicon.multiscreen.server") == false)
                    {
                        LogTool.v("Kill previous application by package name:"
                            + mPrevAppPackageName);
                        killProcessByPkgName(mPrevAppPackageName);
                    }
                    else
                    {
                        LogTool.v("preserve MultiScreen Server");
                    }
                }

                Intent intent = new Intent(Intent.ACTION_MAIN);
                intent.addCategory(Intent.CATEGORY_LAUNCHER);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                ComponentName component = new ComponentName(mPackageName, pkgClassName);
                intent.setComponent(component);
                mContext.startActivity(intent);

                /*
                 * Remember Last APK Name,So that it can be killed On Next
                 * PUSH.<br>?????????????????????????????????????????????????????????????????????????????????????????????
                 */
                mPrevAppPackageName = mPackageName;

            }
            catch (Exception e)
            {
                LogTool.e(e.getMessage());
            }

        }

        /**
         * Kill process by package name.<br>
         * CN:???????????????????????????
         * @param pkgName - package name
         * @return - true if success
         */
        private boolean killProcessByPkgName(String pkgName)
        {
            try
            {
                Class<?> c = Class.forName("android.app.ActivityManagerNative");
                Method getDefaultMethod = c.getMethod("getDefault");
                getDefaultMethod.setAccessible(true);
                Object nativeManager = getDefaultMethod.invoke(null);
                c = nativeManager.getClass();
                Method forceStopPackageMethod = c.getMethod("forceStopPackage", String.class);
                forceStopPackageMethod.setAccessible(true);
                forceStopPackageMethod.invoke(nativeManager, pkgName);
            }
            catch (Exception e)
            {
                LogTool.e(e.getMessage());
                return false;
            }
            return true;
        }

        /**
         * Stop thread.<br>
         * CN:???????????????
         */
        public void destroy()
        {
            if (this.isAlive())
            {
                this.interrupt();
            }
        }
    }

    /**
     * Broadcast switch state for MultiScreenServerActivity.<br>
     * CN:?????????????????????????????????????????????????????????
     * @param isOpen
     */
    private void broadcastSwitchState(boolean isOpen)
    {
        Intent intent;
        if (isOpen)
        {
            intent = new Intent(MultiScreenIntentAction.MULITSCREEN_SERVER_START);
        }
        else
        {
            intent = new Intent(MultiScreenIntentAction.MULITSCREEN_SERVER_STOP);
        }
        sendBroadcast(intent);
    }

    /**
     * Register switch change receiver.<br>
     * CN:????????????????????????????????????
     */
    private void registerSwitchReceiver()
    {
        IntentFilter filter = new IntentFilter();
        filter.addAction(MultiScreenIntentAction.HISILICON_CHANGE_NAME);
        filter.addAction(MultiScreenIntentAction.MULITSCREEN_CHANGE_NAME);
        filter.addAction(MultiScreenIntentAction.MULITSCREEN_SWITCH_OPEN);
        filter.addAction(MultiScreenIntentAction.MULITSCREEN_SWITCH_CLOSE);
        if (mSwitchChangeReceiver == null)
        {
            mSwitchChangeReceiver = new BroadcastReceiver()
            {
                @Override
                public void onReceive(Context context, Intent intent)
                {
                    String action = intent.getAction();
                    if (MultiScreenIntentAction.MULITSCREEN_SWITCH_OPEN.equalsIgnoreCase(action))
                    {
                        // CN:?????????????????????
                        registerNetworkBroadcastReceiver();
                        broadcastSwitchState(true);
                    }
                    else if (MultiScreenIntentAction.MULITSCREEN_SWITCH_CLOSE
                        .equalsIgnoreCase(action))
                    {
                        // CN:?????????????????????
                        unregisterNetworkBroadcastReceiver();
                        stopMultiScreenAll(SENDER_MANUAL_SWITCH);
                        broadcastSwitchState(false);
                    }
                    else if(MultiScreenIntentAction.HISILICON_CHANGE_NAME.equalsIgnoreCase(action)
                        || MultiScreenIntentAction.MULITSCREEN_CHANGE_NAME.equalsIgnoreCase(action))
                    {
                        hostName = intent.getStringExtra("host_name");
                        registerNetworkBroadcastReceiver();
                        broadcastSwitchState(true);
                    }
                }
            };
        }

        registerReceiver(mSwitchChangeReceiver, filter);
    }

    /**
     * Unregister switch change receiver.<br>
     * CN:???????????????????????????????????????
     */
    private void unregisterSwitchReceiver()
    {
        if (mSwitchChangeReceiver != null)
        {
            unregisterBroadcastReceiver(mSwitchChangeReceiver);
            mSwitchChangeReceiver = null;
        }
    }

    /**
     * Register network change broadcast receiver.<br>
     * CN:??????????????????????????????????????????<br>
     * Restart or stop discovery server according to network status.<br>
     * CN:?????????????????????????????????????????????????????????
     */
    private void registerNetworkBroadcastReceiver()
    {
        resetNetworkStateVariable();
        IntentFilter filter = new IntentFilter(WifiManager.WIFI_AP_STATE_CHANGED_ACTION);
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

        mNetworkChangeReceiver = new BroadcastReceiver()
        {
            public void onReceive(Context context, Intent intent)
            {
                if (WifiManager.WIFI_AP_STATE_CHANGED_ACTION.equalsIgnoreCase(intent.getAction())
                    || ConnectivityManager.CONNECTIVITY_ACTION.equalsIgnoreCase(intent.getAction()))
                {
                    // CN:?????????????????????
                    LogTool.d("Receive broadcast: " + intent.getAction());

                    boolean networkstatus = isNetworkEnable();
                    if (networkstatus == true)
                    {
                        // Network on.
                        LogTool.d("network status on.");
                        netOldState = NetStatus.NetworkOn;
                        restartMultiScreenAll(SENDER_NETWORK_CHANGE);
                    }
                    else
                    {
                        // Network off.
                        LogTool.d("network status off");
                        netOldState = NetStatus.NetworkOff;
                        stopMultiScreenAll(SENDER_NETWORK_CHANGE);
                    }
                }
            }
        };

        registerReceiver(mNetworkChangeReceiver, filter);
    }

    /**
     * Unregister receiver of network status changing.<br>
     * CN:??????????????????????????????
     */
    private void unregisterNetworkBroadcastReceiver()
    {
        if (mNetworkChangeReceiver != null)
        {
            unregisterBroadcastReceiver(mNetworkChangeReceiver);
            mNetworkChangeReceiver = null;
        }
        resetNetworkStateVariable();
    }

    private void registerPowerBroadcastReceiver()
    {
        IntentFilter powerIntentFilter = new IntentFilter();
        powerIntentFilter.addAction(SMART_SUSPEND_ENTER);
        // powerIntentFilter.addAction(SMART_SUSPEND_QUIT);
        mPowerReceiver = new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context, Intent intent)
            {
                LogTool.d("receive action " + intent.getAction());
                if (SMART_SUSPEND_ENTER.equalsIgnoreCase(intent.getAction()))
                {
                    mMultiScreenNative.MultiScreenNativeSuspend();
                }
            }
        };
        registerReceiver(mPowerReceiver, powerIntentFilter);
    }

    private void unregisterPowerReceiver()
    {
        if (mPowerReceiver != null)
        {
            unregisterBroadcastReceiver(mPowerReceiver);
            mPowerReceiver = null;
        }
    }

    /**
     * Reset variable of network state.<br>
     * CN:???????????????????????????
     */
    private void resetNetworkStateVariable()
    {
        netOldState = NetStatus.NetworkOff;
    }

    /**
     * Initial PushServer.<br>
     * CN:?????????PushServer???
     */
    private void initPushServer()
    {
        mSaxXmlTool = new SaxXmlUtil();
        mPushServer = new PushServer(PUSHSERVER_SERVICE_PORT);
        mPushServer.setMessageHandler(this);
    }

    /**
     * Initial VIMEControlServer.<br>
     * CN:?????????VIMEControlServer???
     */
    private void initVIMEControlServer()
    {
        mVImeControl =
            new VImeControl(mContext,
                (InputMethodManager) this.getSystemService(Context.INPUT_METHOD_SERVICE));
    }

    /**
     * Initial MultiScreenNative.<br>
     * CN:?????????MultiScreenNative???
     */
    private void initMultiScreenNative()
    {
        mMultiScreenNative = MultiScreenNative.getInstance();
        mMultiScreenNative.setCallback(mNativeCallback);
        mMultiScreenNative.MultiScreenNativeInit();
    }

    /**
     * Restart upnp stack.<br>
     * @return result.
     */
    private int restartUpnpStack()
    {
        stopUpnpStack();
        int retValue = startUpnpStack();
        return retValue;
    }

    /**
     * Start UPNP stack.<br>
     * @return result.
     */
    private int startUpnpStack()
    {
        int retValue = 0;

        if ((isUpnpStackOpen == false) && (netOldState == NetStatus.NetworkOn))
        {
            LogTool.d("startUpnpStack.");
            isUpnpStackOpen = true;
            LogTool.i("start with net type " + mActiveNetType);
            String mHostName = null;
            if(hostName == null)
                mHostName = readFriendlyName();
            else
                mHostName = hostName;

            retValue =
                mMultiScreenNative.MultiScreenDeviceStart(
                    UpnpMultiScreenDeviceInfo.defaultServiceList(), mHostName,
                    mActiveNetType);
            writeMultiScreenStatusPreference("1");
            /*
             *  close scene because no one use it
             *  if open scene and not call this fun, will cause memory error
             */
            //initSceneObserver();
        }

        return retValue;
    }

    /**
     * Stop UPNP stack.<br>
     * @return result.
     */
    private int stopUpnpStack()
    {
        LogTool.d("stopUpnpStack.");
        isUpnpStackOpen = false;
        /*
         *  close scene because no one use it
         *  if open scene and not call this fun, will cause memory error
         */
        //deinitSceneObserver();
        int retValue = mMultiScreenNative.MultiScreenDeviceStop();
        writeMultiScreenStatusPreference("0");
        return retValue;
    }

    /**
     * Notify client that server is manual off.
     * @return result
     */
    private int notifyManualOff()
    {
        LogTool.d("Notify client that server is manual off.");
        int retValue = mMultiScreenNative.MultiScreenNotifyManualOff();
        return retValue;
    }

    /**
     * Start VIME Control server.<br>
     * CN:??????VIME ???????????????
     * @return result O means execute method success,-1 means execute method
     *         failure.<br>
     *         CN:0???????????????????????????-1???????????????????????????
     */
    private int startVIMEControl()
    {
        if (!mIsVIMEControlServerStart)
        {
            LogTool.d("startVIMEControl");
            mVImeControl.startControl();
            mIsVIMEControlServerStart = true;
        }
        else
        {
            LogTool.i("VIME Control server has started");
        }
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    /**
     * Stop VIME Control Server.<br>
     * CN:??????VIME??????????????????
     * @return result O means execute method success,-1 means execute method
     *         failure.<br>
     *         CN:0???????????????????????????-1???????????????????????????
     */
    private int stopVIMEControl()
    {
        if (mIsVIMEControlServerStart)
        {
            LogTool.d("stop VIMEControl");
            mVImeControl.stopControl();
            mIsVIMEControlServerStart = false;
        }
        else
        {
            LogTool.i("VIME Control server has stopped");
        }
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    /**
     * Set VIME Client's url <br>
     * CN:??????VIME Client???s url.
     * @param url VIME client's url.<br>
     *        CN:VIME????????????url???
     * @return result O means execute method success,-1 means execute method
     *         failure.<br>
     *         CN:0???????????????????????????-1???????????????????????????
     */
    private int setVIMEClientURL(String url)
    {
        LogTool.d("setVIMEClientURL");
        mVImeControl.setClientURL(url);
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    /**
     * Start remote app server.<br>
     * CN:??????remote app ?????????
     * @return result O means execute method success,-1 means execute method
     *         failure.<br>
     *         CN:0???????????????????????????-1???????????????????????????
     */
    private int startRemoteApp()
    {
        if (!mIsRemoteAppServerStart)
        {
            mPushServer.start();
            mIsRemoteAppServerStart = true;
            LogTool.d("remote app started");
        }
        else
        {
            LogTool.i("RemoteApp server has stopped");
        }
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    /**
     * Stop remote app server.<br>
     * CN:??????remote app ?????????
     * @return result O means execute method success,-1 means execute method
     *         failure.<br>
     *         CN:0???????????????????????????-1???????????????????????????
     */
    private int stopRemoteApp()
    {
        if (mIsRemoteAppServerStart)
        {
            mPushServer.stop();
            mIsRemoteAppServerStart = false;
            LogTool.d("remote app stopped");
        }
        else
        {
            LogTool.i("RemoteApp server has stopped");
        }
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    private int notifySuspendQuit()
    {
        LogTool.d("notifySuspendQuit");
        Intent intent = new Intent(SMART_SUSPEND_QUIT);
        mContext.sendBroadcast(intent);
        return MultiScreenNativeCommand.COMMAND_RET_SUCCESS;
    }

    /**
     * Return whether the network is up. ????????????????????????
     */
    private boolean isNetworkEnable()
    {
        ConnectivityManager networkManager =
            (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (networkManager == null)
        {
            LogTool.e("get connectivityManager is null");
            mActiveNetType = NIC_DEFAULT_NAME;
            return false;
        }
        EthernetManager ethManager =
            (EthernetManager) mContext.getSystemService(Context.ETHERNET_SERVICE);
        if (ethManager == null)
        {
            LogTool.e("get ethernetManager is null");
            mActiveNetType = NIC_DEFAULT_NAME;
            return false;
        }
        mActiveNetType = NIC_DEFAULT_NAME;


        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        if ( (wifiManager != null)&& (wifiManager.getWifiApState() == WifiManager.WIFI_AP_STATE_ENABLED))
        {
            LogTool.i("AP is open");
            mActiveNetType = WIFI_NIC_NAME;
        }

        if (networkManager.getActiveDefaultNetwork() == ConnectivityManager.TYPE_WIFI)
        {
            LogTool.i("wifi is open");
            mActiveNetType = WIFI_NIC_NAME;
        }
        else if (networkManager.getActiveDefaultNetwork() == ConnectivityManager.TYPE_ETHERNET) {
            NetworkInfo ni = networkManager.getWifiStateWithoutDisguise();
            if(ni.isConnected() == true) {
            //wifi???ethernet?????????
                LogTool.i("both ethernet and wifi is open");
                if(DEFAULT_USE_ADAPTER_WIFI_ETHERNET.contains("wlan"))
                {
                   mActiveNetType = WIFI_NIC_NAME;
                }
                else
                {
                   mActiveNetType = getActiveNetType(ethManager);
                }

            } else {
            //??????ethernet??????
                LogTool.i("ethernet is open");
                mActiveNetType = getActiveNetType(ethManager);//"eth0";
            }
        }

        LogTool.i("mActiveNetType " + mActiveNetType);

        if (mActiveNetType.equalsIgnoreCase(NIC_DEFAULT_NAME))
            return false;
        else
            return true;
    }

    /**
     * Get active net type.<br>
     * @return
     */
    private String getActiveNetType(EthernetManager ethManager)
    {
        String netType = NIC_DEFAULT_NAME;
        Method method = null;
        Class<?> c = null;
        try
        {
            c = Class.forName("android.net.ethernet.EthernetManager");
        }
        catch (ClassNotFoundException e)
        {
            LogTool.e(e.getMessage());
            return netType;
        }

        try
        {
            method = c.getMethod("getDatabaseInterfaceName");
        }
        catch (NoSuchMethodException e)
        {
            LogTool.e(e.getMessage());
            try
            {
                method = c.getMethod("getInterfaceName");
            }
            catch (NoSuchMethodException e1)
            {
                LogTool.e(e1.getMessage());
            }
        }

        if (method != null)
        {
            try
            {
                // netType = (String) method.invoke(c.newInstance());
                netType = (String) method.invoke(ethManager);
            }
            catch (InvocationTargetException e)
            {
                LogTool.e(e.getMessage());
            }
            catch (IllegalAccessException e)
            {
                LogTool.e(e.getMessage());
            }
        }

        return netType;
    }

    private void restartMultiScreenAll(int senderId)
    {
        clearMessages();
        if (myHandler != null)
        {
            Message msg = myHandler.obtainMessage();
            if (msg == null)
            {
                LogTool.e("Obtain msg failed.");
                return;
            }
            msg = myHandler.obtainMessage();
            msg.what = MSG_RESTART_MULTISCREEN;
            myHandler.sendMessageDelayed(msg, DELAY_MILLIS_HANDLE);
        }
    }

    private void writeMultiScreenStatusPreference(String mStatus)
    {
        SharedPreferences.Editor editor =
            getSharedPreferences("MultiScreenSetting", Context.MODE_WORLD_READABLE|Context.MODE_MULTI_PROCESS).edit();
        editor.putString("MultiScreenStatus", mStatus);
        editor.commit();
    }

    private void stopMultiScreenAll(int senderId)
    {
        clearMessages();
        if (myHandler != null)
        {
            Message msg = myHandler.obtainMessage();
            if (msg == null)
            {
                LogTool.e("Obtain msg failed.");
                return;
            }
            else
            {
                msg.what = MSG_STOP_MULTISCREEN;
            }

            switch (senderId)
            {
                case SENDER_NETWORK_CHANGE:
                {
                    myHandler.sendMessageDelayed(msg, DELAY_MILLIS_HANDLE);
                }
                    break;

                case SENDER_MANUAL_SWITCH:
                {
                    notifyManualOff();
                    myHandler.sendMessageDelayed(msg, DELAY_MILLIS_MANUAL_OFF);
                }
                    break;

                default:
                    break;
            }
        }
    }

    private void clearMessages()
    {
        if (myHandler != null)
        {
            if (myHandler.hasMessages(MSG_RESTART_MULTISCREEN))
                myHandler.removeMessages(MSG_RESTART_MULTISCREEN);

            if (myHandler.hasMessages(MSG_STOP_MULTISCREEN))
                myHandler.removeMessages(MSG_STOP_MULTISCREEN);
        }
    }

    private class MyHandler extends Handler
    {
        public MyHandler(Looper looper)
        {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg)
        {
            switch (msg.what)
            {
                case MSG_RESTART_MULTISCREEN:
                {
                    if (stateManager.isOpen())
                    {
                        restartUpnpStack();
                    }
                }
                    break;

                case MSG_STOP_MULTISCREEN:
                {
                    stopUpnpStack();
                }
                    break;

                case MSG_START_SPEECH:
                {
                    showSpeechDialog();
                }
                    break;

                case SpeechDialog.MSG_UPDATE_SPEECH_VIEW:
                {
                    updateSpeechContent();
                }
                    break;

                case SpeechDialog.MSG_UPDATE_WEATHER_VIEW:
                {
                    Bundle data = msg.getData();
                    String msgValue = data.getString("value");

                    updateWeatherContent(msgValue);
                }
                    break;

                case SpeechDialog.MSG_KEYDOWN_BACK:
                case SpeechDialog.MSG_DIALOG_TIMEOUT:
                case SpeechDialog.MSG_SPEECH_ERROR:
                case SpeechDialog.MSG_SPEECH_DIALOG_DISMISS:
                {
                    dismissSpeechDialog();
                }
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Unregister receiver of broadcast.<br>
     * CN:???????????????????????????
     * @param receiver broadcast receiver.
     */
    private void unregisterBroadcastReceiver(BroadcastReceiver receiver)
    {
        try
        {
            unregisterReceiver(receiver);
        }
        catch (IllegalArgumentException e)
        {
            LogTool.d("the receiver was already unregistered or was not registered.");
        }
    }

    /**
     * Is soft AP enable.<br>
     * CN: ???AP???????????????
     * @return true???ap
     */
    private boolean isSoftAPEnable()
    {
        WifiManager wifiMgr = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        if (null == wifiMgr)
        {
            LogTool.e("wifiManager is null");
            return false;
        }
        LogTool.d("isWifiApEnabled: " + wifiMgr.isWifiApEnabled());
        if (wifiMgr.isWifiApEnabled())
        {
            return true;
        }
        return false;
    }

    /**
     * Show speech Dialog.
     */
    private void showSpeechDialog()
    {
        if (mSpeechDialog == null)
        {
            mSpeechDialog = new SpeechDialog(mContext, R.style.SpeechDialogStyle, myHandler);
        }

        DialogUtil.showServiceDialog(mSpeechDialog);
    }

    /**
     * Dismiss speech Dialog.
     */
    private void dismissSpeechDialog()
    {
        if (mSpeechDialog != null)
        {
            mSpeechDialog.dismiss();
        }
    }

    /**
     * Update speech content.
     */
    private void updateSpeechContent()
    {
        if (mSpeechDialog != null)
        {
            mSpeechDialog.updateSpeechAdapter();
        }
    }

    /**
     * Update weather content.
     */
    private void updateWeatherContent(String msgValue)
    {
        if (mSpeechDialog != null)
        {
            mSpeechDialog.updateWeatherData(msgValue);
        }
    }
}
