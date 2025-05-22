
import {  mediaquery, } from '@kit.ArkUI';
import PPSSPPService from './PPSSPPService';
import emitter from '@ohos.events.emitter';


interface  XComponentAttrs{
  id: string
  type: XComponentType
  libraryname: string
}

@Entry
@Component
struct Index {
  @State portraitFunc: mediaquery.MediaQueryResult | void | null = null;
	@State showSheet:boolean = false;
	@State inputText:string = "";
	@State requestId: number = 0;
	@State title: string = "";
  onBackPress() {
    return PPSSPPService.onBackPress()
  }
  aboutToAppear(): void {
		emitter.on({eventId: 100000},(res)=>{
			this.showSheet = true
			const paramString = res?.data?.["params"] as string
			const param = paramString.split(":@:", 3);
			const requestID =	parseInt(param[0]);
			if (param.length > 1 && param[1] != "")
				this.title = param[1];
			if (param.length > 2)
				this.inputText = param[2];
			this.requestId = requestID;
		})
  }
  xComponentAttrs: XComponentAttrs = {
    id: 'xcomponentId1',
    type: XComponentType.SURFACE,
    libraryname: 'entry'
  }
	@Builder InputBox(context: UIContext){
		Column(){
			Blank().height(20)
			Row(){
				TextInput({text: this.inputText, placeholder: "请输入"}).onChange((value)=>{
					this.inputText = value
				})
					.key("input")
					.enterKeyType(EnterKeyType.Done)
					.onSubmit(()=>{
						PPSSPPService.sendRequestResult(this.requestId, true, this.inputText, 0);
						this.showSheet = false;
					})
					.onAppear( () => {
						let fc = context.getFocusController()
						fc?.requestFocus("input")
				}).layoutWeight(1)
				Blank().width(10)
				Button({ type: ButtonType.Capsule, stateEffect: true }) {
					Row() {
						Text('确定').fontSize(12).fontColor(0xffffff).textAlign(TextAlign.Center).layoutWeight(1)
					}.alignItems(VerticalAlign.Center).width(100).height(40)
				}.onClick(async () => {
					PPSSPPService.sendRequestResult(this.requestId, true, this.inputText, 0);
					this.showSheet = false;
				})
			}.padding(10)
		}
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
		.bindSheet(this.showSheet,
			this.InputBox( this.getUIContext()), {
				height: 300,
				dragBar: true,
				title: {title: this.title},
				shouldDismiss:()=> {
					PPSSPPService.sendRequestResult(this.requestId, false, "", 0);
					this.showSheet = false
				}
			})
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.BOTTOM, SafeAreaEdge.TOP])
  }
}
