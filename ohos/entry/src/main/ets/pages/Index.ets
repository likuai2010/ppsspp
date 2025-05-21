import { hilog } from '@kit.PerformanceAnalysisKit';

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

  aboutToAppear(): void {

  }


  xComponentAttrs: XComponentAttrs = {
    id: 'xcomponentId1',
    type: XComponentType.SURFACE,
    libraryname: 'entry'
  }
  build() {
    Stack({alignContent: Alignment.Center}) {
      XComponent(this.xComponentAttrs).onLoad(() => {
				// 必须调用
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
