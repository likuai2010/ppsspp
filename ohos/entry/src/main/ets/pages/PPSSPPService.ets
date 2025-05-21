import { display, promptAction, window } from '@kit.ArkUI';
import { common, Context, UIAbility } from '@kit.AbilityKit';
import napi,{ NativeApi } from 'libppsspp_core.so'
import { picker } from '@kit.CoreFileKit';

const ppsspp = napi as NativeApi

class PPSSPPService{
	context?: Context
	init(context: Context){
		this.context = context
		ppsspp.native_init(context.resourceDir, context.filesDir, context.cacheDir)
		ppsspp?.onPostCommand((c,p)=>{ this.processCommand(c,p) })
		this.updateDisplayParameters()

		setTimeout(()=>{
			let ac = context as common.UIAbilityContext
			let windows = ac.windowStage.getMainWindowSync()
			windows.setWindowSystemBarEnable([])
			windows.on('windowSizeChange', async  () => {
				this.sendSafeInsets()
			});
			this.changeOrientation("0")
		}, 2000)
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
				let documentPicker = new picker.DocumentViewPicker(this.context!);
				let documentSelectOptions = new picker.DocumentSelectOptions();
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
				}, 2000)

			}
			if (command === "finish" ) {
				this.context?.getApplicationContext().killAllProcesses()
			}
			if(command == 'rotate'){
				const rotString = ppsspp.queryConfig("screenRotation")
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
}



export default  new PPSSPPService()

