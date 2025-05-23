import { AbilityConstant, common, Context, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { promptAction, window } from '@kit.ArkUI';
import fs from '@ohos.file.fs';
import PPSSPPService, { getDownloadDir } from '../pages/PPSSPPService';
import { abilityAccessCtrl, bundleManager, Permissions } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';

async function checkPermissionGrant(permission: Permissions): Promise<abilityAccessCtrl.GrantStatus> {
	let atManager: abilityAccessCtrl.AtManager = abilityAccessCtrl.createAtManager();
	let grantStatus: abilityAccessCtrl.GrantStatus = abilityAccessCtrl.GrantStatus.PERMISSION_DENIED;

	// 获取应用程序的accessTokenID。
	let tokenId: number = 0;
	try {
		let bundleInfo: bundleManager.BundleInfo = await bundleManager.getBundleInfoForSelf(bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_APPLICATION);
		let appInfo: bundleManager.ApplicationInfo = bundleInfo.appInfo;
		tokenId = appInfo.accessTokenId;
	} catch (error) {
		const err: BusinessError = error as BusinessError;
		console.error(`Failed to get bundle info for self. Code is ${err.code}, message is ${err.message}`);
	}

	// 校验应用是否被授予权限。
	try {
		grantStatus = await atManager.checkAccessToken(tokenId, permission);
	} catch (error) {
		const err: BusinessError = error as BusinessError;
		console.error(`Failed to check access token. Code is ${err.code}, message is ${err.message}`);
	}

	return grantStatus;
}

async function checkPermissions(): Promise<void> {
	let grantStatus1: boolean = await checkPermissionGrant('ohos.permission.READ_WRITE_USER_FILE') === abilityAccessCtrl.GrantStatus.PERMISSION_GRANTED;
	let grantStatus2: boolean = await checkPermissionGrant('ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY') === abilityAccessCtrl.GrantStatus.PERMISSION_GRANTED;
	// 精确定位权限只能跟模糊定位权限一起申请，或者已经有模糊定位权限才能申请精确定位权限。
	if (grantStatus2 && !grantStatus1) {
		// 申请精确定位权限。
	} else if (!grantStatus1 && !grantStatus2) {
		// 申请模糊定位权限与精确定位权限或单独申请模糊定位权限。
	} else {
		// 已经授权，可以继续访问目标操作。
	}
}
function reqPermissionsFromUser(permissions: Array<Permissions>, context: common.UIAbilityContext): void {
	let atManager: abilityAccessCtrl.AtManager = abilityAccessCtrl.createAtManager();
	atManager.requestPermissionsFromUser(context, permissions).then((data) => {
		let grantStatus: Array<number> = data.authResults;
		let length: number = grantStatus.length;
		for (let i = 0; i < length; i++) {
			if (grantStatus[i] === 0) {
			} else {
				return;
			}
		}
		// 授权成功
	})
}


export async  function readFile(filePath: string): Promise<Uint8Array | null> {
  if(!await fs.access(filePath))
    return null;
  const file = await fs.open(filePath)
  const stats = await fs.stat(filePath)
  let bufSize = stats.size;
  let buf = new ArrayBuffer(bufSize);
  await fs.read(file.fd, buf, { offset: 0, length: bufSize });
  await fs.close(file);
  return new Uint8Array(buf);
}
export function writeFile(filePath: string, data: Uint8Array | null) {
  if(data != null && data.byteLength > 0){
    const file = fs.openSync(filePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE)
    fs.writeSync(file.fd, data.buffer);
    fs.fsyncSync(file.fd)
    fs.closeSync(file);
  }
}




export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onCreate');
		PPSSPPService.init(this.context)
		setTimeout(()=>{
			let uri = want.uri;
			if (uri && uri != "") {
				PPSSPPService.loadFileUrl(uri)
			}
		}, 3000)
  }
	async onNewWant(want: Want, launchParam: AbilityConstant.LaunchParam): Promise<void> {
		super.onNewWant(want, launchParam)
		let uri = want.uri;
		if (uri && uri != "") {
			PPSSPPService.loadFileUrl(uri)
		}
	}


  async initApp(windowStage: window.WindowStage){

    windowStage.loadContent('pages/Index', async  (err) => {
			//reqPermissionsFromUser(['ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY', 'ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY'], this.context);
			try {
				await PPSSPPService.authDownloadDir();
			}catch (e) {
				promptAction.showToast({message: "download授权失败"})
			}
			if (err.code) {
        hilog.error(0x0000, 'testTag', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err) ?? '');
        return;
      }
      //
    });
  }
  onWindowStageCreate(windowStage: window.WindowStage): void {
    // Main window is created, set main page for this ability
    this.initApp(windowStage)
  }

  onWindowStageDestroy(): void {
    // Main window is destroyed, release UI related resources
    hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    // Ability has brought to foreground
    hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    // Ability has back to background
    hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onBackground');
  }
};
