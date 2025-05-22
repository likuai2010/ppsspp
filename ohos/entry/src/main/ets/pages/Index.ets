import { hilog } from '@kit.PerformanceAnalysisKit';

import { display, mediaquery, promptAction, window } from '@kit.ArkUI';
import { InputAxis, translateAxis, translateController, translateKeyboard } from './constants.ts';
import PPSSPPService from './PPSSPPService';

interface  XComponentAttrs{
  id: string
  type: XComponentType
  libraryname: string
}
@Entry
@Component
struct Index {
  @State portraitFunc:mediaquery.MediaQueryResult|void|null = null;
  onBackPress() {
  	PPSSPPService.onBackPress()
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
			PPSSPPService.onEvent(e)
			e.stopPropagation();
    }).onFocusAxisEvent((e)=>{
			PPSSPPService.onFocusAxisEvent(e)
			e.stopPropagation();
    })
    .height('100%')
    .height('100%')
    .backgroundColor(Color.Black)
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM, SafeAreaEdge.TOP])
  }
}
