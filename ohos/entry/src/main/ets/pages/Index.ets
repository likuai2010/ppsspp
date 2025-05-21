import { hilog } from '@kit.PerformanceAnalysisKit';

import picker from '@ohos.file.picker';
import common from '@ohos.app.ability.common'
import { display, mediaquery, promptAction, window } from '@kit.ArkUI';
import { NativeApi } from 'libentry.so'
import { InputAxis, translateAxis, translateController, translateKeyboard } from './constants.ts';

interface  XComponentAttrs{
  id: string
  type: XComponentType
  libraryname: string
}
@Entry
@Component
struct Index {
  private xComponentContext: NativeApi | undefined;
  @State portraitFunc:mediaquery.MediaQueryResult|void|null = null;
  onBackPress() {
    if(this.xComponentContext){
      if(!this.xComponentContext.isAtTopLevel()){
        this.xComponentContext?.keyDown(0, 4, false)
        this.xComponentContext?.keyUp(0, 4)
        return true;
      }
    }
    return false;
  }
  processCommand(command:string, params:string){
    console.log("onPostCommand: ", command, params)
    if (command.startsWith("launchBrowser")) {
      return;
    }
    if (command.startsWith("launchEmail")) {
      return;
    }
    if(command.startsWith("browse_file") || command.startsWith("browse_file_audio") || command.startsWith("browse_file_zip")){
      let context = getContext(this) as common.Context; // 请确保getContext(this)返回结果为UIAbilityContext
      let documentPicker = new picker.DocumentViewPicker(context);
      let documentSelectOptions = new picker.DocumentSelectOptions();
      documentPicker.select(documentSelectOptions).then((documentSelectResult)=>{
        if(documentSelectResult.length > 0){
          this.xComponentContext?.sendRequestResult(parseInt(params), true, documentSelectResult[0], 0)
        }
      })
    }
    if(command == "toast"){
        promptAction.showToast({message: params})
    }
    if (command === "recreate") {
      let context = getContext(this) as common.Context;
      // setTimeout(()=>{
      //   context.getApplicationContext().restartApp({
      //     bundleName: 'com.xiaobai.ppsspp',
      //     abilityName: 'EntryAbility'
      //   })
      // }, 2000)
    }
    if (command === "restart") {
      let context = getContext(this) as common.Context;
      setTimeout(()=>{
        context.getApplicationContext().restartApp({
          bundleName: 'com.xiaobai.ppsspp',
          abilityName: 'EntryAbility'
        })
      }, 2000)

    }
    if (command === "finish" ) {
      let context = getContext(this) as common.Context;
      context.getApplicationContext().killAllProcesses()
    }
    if(command == 'rotate'){
      const rotString = this.xComponentContext?.queryConfig("screenRotation")
      this.changeOrientation(rotString)
    }
    if(command.startsWith("browse_folder")){
      let context = getContext(this) as common.Context; // 请确保getContext(this)返回结果为UIAbilityContext
      let documentPicker = new picker.DocumentViewPicker(context);
      let documentSelectOptions = new picker.DocumentSelectOptions();
      documentPicker.select(documentSelectOptions).then((documentSelectResult)=>{
        if(documentSelectResult.length > 0){
          this.xComponentContext?.sendRequestResult(parseInt(params), true, documentSelectResult[0], 0)
        }
      })
    }
  }
  checkDisplayParameters(){
    let displayClass: display.Display | null = null;
    try {
      displayClass = display.getDefaultDisplaySync();
      if(this.xComponentContext){
        this.xComponentContext.setDisplayParameters(displayClass?.width, displayClass?.height, displayClass?.densityDPI, displayClass?.refreshRate)
      }
      this.sendSafeInsets()
    } catch (exception) {
      console.error('Failed to obtain the default display object. Code: ' + JSON.stringify(exception));
    }
  }
  aboutToAppear(): void {

    window.getLastWindow(getContext(this), (err, data) => {
      data.on('windowSizeChange', async  (size) => {
        this.sendSafeInsets()
      });
    });
  }
  async sendSafeInsets(){
    const displayClass = display.getDefaultDisplaySync();
    const cutoutInfo = await displayClass.getCutoutInfo()
    if (cutoutInfo.boundingRects.length > 0){
      let top = 0;
      let bottom = 0;
      let left = 0;
      let right = 0;
      let out = cutoutInfo.boundingRects[0]
      if (displayClass.rotation == 0){
        top = out.top + out.height;
      } else if(displayClass.rotation == 1){
        left = out.width;
      } else if(displayClass.rotation == 2){
       // right = out.width;
      } else {
        left = out.width;
      }
      console.log(`Safe Area Insets:
                Top: ${top},
                Bottom: ${bottom},
                Left: ${left},
                Right: ${right}`);
      this.xComponentContext?.sendMessage("safe_insets", `${left}:${right}:${top}:${bottom}`)
    }
  }
  onPageShow(): void {
    this.checkDisplayParameters()
    this.changeOrientation("0");
  }



  onInit(){
    if(this.xComponentContext){
      this.xComponentContext?.onPostCommand((c,p)=>{ this.processCommand(c,p) })
      this.checkDisplayParameters();
      const context = getContext(this);
      //this.xComponentContext.native_init(context.resourceDir, context.filesDir, context.cacheDir)
    }
  }

  private changeOrientation(rotString: string | undefined) {
    let ori = window.Orientation.AUTO_ROTATION;
    switch ( parseInt(rotString|| "0")){
      case 0:
        ori = window.Orientation.AUTO_ROTATION;
        break;
      case 1:
        ori = window.Orientation.LANDSCAPE;
        break;
      case 2:
        ori = window.Orientation.PORTRAIT;
        break;
      case 3:
        ori = window.Orientation.LANDSCAPE_INVERTED;
        break;
      case 4:
        ori = window.Orientation.PORTRAIT_INVERTED;
        break;
      case 5:
        ori = window.Orientation.AUTO_ROTATION_LANDSCAPE;
        break;
    }
    // 获取UIAbility实例的上下文信息
    let context:common.UIAbilityContext = getContext(this) as common.UIAbilityContext;
    // 调用该接口手动改变设备横竖屏状态
    window.getLastWindow(context).then((lastWindow) => {
      lastWindow.setPreferredOrientation(ori)
    });
  }
  xComponentAttrs: XComponentAttrs = {
    id: 'xcomponentId1',
    type: XComponentType.SURFACE,
    libraryname: 'entry'
  }
  build() {
    Stack({alignContent: Alignment.Center}) {
      XComponent(this.xComponentAttrs).onLoad((xComponentContext?: object | NativeApi) => {
        if (xComponentContext) {
          this.xComponentContext = xComponentContext as NativeApi;
          this.onInit();
        }
      })
      .backgroundColor(Color.Black)
      .width('100%')
      .height('100%')
    }
		.key("onKeyEvent")
		.focusable(true)
		.defaultFocus(true)
    .onKeyEvent((e)=>{
      // DEVICE_ID_KEYBOARD 1
      let code = 0;
      let deviceId = 0;
      if(e.keySource == KeySource.Keyboard){
        deviceId = 1
        code = translateKeyboard(e.keyCode);
      }else {
				deviceId = e.deviceId
        code = translateController(e.keyCode);
      }
      if(e.type == KeyType.Up){
        this.xComponentContext?.keyUp(deviceId, code);
      }else{
        this.xComponentContext?.keyDown(deviceId, code, false);
      }
      e.stopPropagation()
    }).onFocusAxisEvent((event)=>{
      // DEVICE_ID_PAD_0 10
			let deviceId = event.deviceId
      let axisIds: number[] = []
      let values: number[] = []
      for(const key of event.axisMap.keys()){
        const id = translateAxis(key)
        const value = event.axisMap.get(key)
        if(value){
          axisIds.push(id)
          values.push(value)
        }
      }

      this.xComponentContext?.joystickAxis(deviceId, axisIds, values, axisIds.length);
      event.stopPropagation()
    })
    .height('100%')
    .height('100%')
    .backgroundColor(Color.Black)
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM, SafeAreaEdge.TOP])
  }
}
