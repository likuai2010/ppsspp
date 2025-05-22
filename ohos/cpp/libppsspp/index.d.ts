
export interface NativeApi{
  native_init: (assetsPath: string, appPath: string, cachePath: string) => void;
  sendMessage: (msg: string, prm: string) => void;
  onPostCommand:(cal:(c: string, p: string )=>void) => void;
  sendRequestResult:(seqID: number, result: boolean, value: string, iValue: number) => void;
  setDisplayParameters:(display_xres: number , display_yres: number,dpi:number , refreshRate: number) =>void;
  backbufferResize:(bufferWidth: number, bufferHeight: boolean , format:number ) =>void;
  keyDown:(deviceId: number, key:number, isRepeat:boolean)=>boolean
  keyUp:(deviceId: number, key:number ) => boolean
  joystickAxis:(deviceId: number, axisIds: number[], values: number[], count: number ) => boolean
  mouseWheelEvent:(x: number, y: number ) => boolean
  queryConfig:(queryName: string) => string;
  isAtTopLevel:() =>boolean;
}
