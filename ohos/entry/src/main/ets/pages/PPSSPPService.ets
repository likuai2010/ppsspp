import { display, promptAction, window } from '@kit.ArkUI';
import { common, Context, UIAbility, Want } from '@kit.AbilityKit';
import napi, { NativeApi } from 'libppsspp_core.so'
import { picker } from '@kit.CoreFileKit';
import { translateAxis, translateController, translateKeyboard } from './constants.ts';
import { emitter } from '@kit.BasicServicesKit';
import { Event } from '@kit.ArkTS';

const ppsspp = napi as NativeApi


export async function toBrowsable(context: Context, uri: string = "https://moonlight-xiaobai.com/query/student") {
	try {
		let want: Want = {
			// uncomment line below if wish to implicitly query only in the specific bundle.
			// bundleName: "com.example.myapplication",
			"action": "ohos.want.action.viewData",
			// entities can be omitted.
			"entities": [ "entity.system.browsable" ],
			"uri": uri,
			"type": "text/plain"
		}
		await (context as common.UIAbilityContext).startAbility(want)
	} catch (error) {
	}
}

class PPSSPPService{
	context?: Context
	init(context: Context){
		this.context = context
		ppsspp.native_init(context.resourceDir, context.filesDir, context.cacheDir)
		ppsspp?.onPostCommand((c:string, p: string)=>{ this.processCommand(c,p) })
		this.updateDisplayParameters()

		setTimeout(()=>{
			let ac = context as common.UIAbilityContext
			let windows = ac.windowStage.getMainWindowSync()
			windows.setWindowSystemBarEnable([])
			windows.on('windowSizeChange', async  () => {
				this.sendSafeInsets()
			});
			this.changeOrientation("0")
		}, 3000)
	}
	sendRequestResult(seqID: number, result: boolean, value: string, iValue: number){
		ppsspp.sendRequestResult(seqID, result, value, iValue)
	}
	processCommand(command:string, params: string){
			promptAction.showToast({message: "cmd: " + command})
			if (command.startsWith("inputbox")) {
				emitter.emit({eventId: 100000}, {data: {params}});
				return;
			}
			if (command.startsWith("browse_image")) {
				let documentPicker = new picker.DocumentViewPicker(this.context!);
				let documentSelectOptions = new picker.DocumentSelectOptions();
				documentSelectOptions.maxSelectNumber = 1;
				documentPicker.select(documentSelectOptions).then((documentSelectResult)=>{
					if(documentSelectResult.length > 0){
						ppsspp?.sendRequestResult(parseInt(params), true, documentSelectResult[0], 0)
					}
				})
				return;
			}
			if (command.startsWith("camera_command")) {

				return;
			}
			if (command.startsWith("share_text")) {
				return;
			}
			if (command.startsWith("launchBrowser")) {
				toBrowsable(this.context!, params)
				return;
			}

			if(command.startsWith("browse_file") || command.startsWith("browse_file_audio") || command.startsWith("browse_file_zip")){
				let documentPicker = new picker.DocumentViewPicker(this.context!);
				let documentSelectOptions = new picker.DocumentSelectOptions();
				documentSelectOptions.maxSelectNumber = 1;
				documentPicker.select(documentSelectOptions).then((documentSelectResult)=>{
					if(documentSelectResult.length > 0){
						ppsspp?.sendRequestResult(parseInt(params), true, documentSelectResult[0], 0)
					}
				})
			}
			if(command == "toast"){
				promptAction.showToast({message: params})
			}
			if (command === "recreate") {
				// setTimeout(()=>{
				//   context.getApplicationContext().restartApp({
				//     bundleName: 'com.xiaobai.ppsspp',
				//     abilityName: 'EntryAbility'
				//   })
				// }, 2000)
			}
			if (command === "restart") {
				setTimeout(()=>{
					this.context?.getApplicationContext().restartApp({
						bundleName: 'com.xiaobai.ppsspp',
						abilityName: 'EntryAbility'
					})
				}, 500)

			}
			if (command === "finish" ) {
				this.context?.getApplicationContext().killAllProcesses()
			}
			if(command == 'rotate'){
				const  rotString: string = ppsspp.queryConfig("screenRotation")
				this.changeOrientation(rotString)
			}
			if(command.startsWith("browse_folder")){
				let documentPicker = new picker.DocumentViewPicker(this.context!);
				let documentSelectOptions = new picker.DocumentSelectOptions();

				documentPicker.select(documentSelectOptions).then((documentSelectResult)=>{
					if(documentSelectResult.length > 0){
						ppsspp?.sendRequestResult(parseInt(params), true, documentSelectResult[0], 0)
					}
				})
			}
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
			ppsspp?.sendMessage("safe_insets", `${left}:${right}:${top}:${bottom}`)
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
		// 调用该接口手动改变设备横竖屏状态
		 window.getLastWindow(this.context).then((lastWindow) => {
			lastWindow.setPreferredOrientation(ori)
		});
	}

	updateDisplayParameters(){
		let displayClass: display.Display | null = null;
		try {
				displayClass = display.getDefaultDisplaySync();
				ppsspp.setDisplayParameters(displayClass?.width, displayClass?.height, displayClass?.densityDPI, displayClass?.refreshRate)
		} catch (exception) {
			console.error('Failed to obtain the default display object. Code: ' + JSON.stringify(exception));
		}
	}
	onEvent(event: KeyEvent){
		// DEVICE_ID_KEYBOARD 1
		let code = 0;
		let deviceId = 0;
		if(event.keySource == KeySource.Keyboard){
			deviceId = 1
			code = translateKeyboard(event.keyCode);
		}else {
			deviceId = event.deviceId
			code = translateController(event.keyCode);
		}
		if(event.type == KeyType.Up){
			ppsspp.keyUp(deviceId, code);
		}else{
			ppsspp.keyDown(deviceId, code, false);
		}
	}
	onFocusAxisEvent(event: FocusAxisEvent){
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
		ppsspp.joystickAxis(deviceId, axisIds, values, axisIds.length);
	}
	onBackPress() : boolean{
		if(!ppsspp.isAtTopLevel()){
			ppsspp?.keyDown(0, 4, false)
			ppsspp?.keyUp(0, 4)
			return true;
		}
		return false;
	}
}



export default  new PPSSPPService()

