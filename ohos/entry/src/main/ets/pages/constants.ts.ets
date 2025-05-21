import { KeyCode } from "@kit.InputKit";

export enum InputDeviceID {
  DEVICE_ID_ANY = -1,  // Represents any device ID
  DEVICE_ID_DEFAULT = 0,  // Old Android
  DEVICE_ID_KEYBOARD = 1,  // PC keyboard, android keyboards
  DEVICE_ID_MOUSE = 2,  // PC mouse only (not touchscreen!)
  DEVICE_ID_PAD_0 = 10,  // Generic joypads
  DEVICE_ID_PAD_1 = 11,  // these should stay as contiguous numbers
  DEVICE_ID_PAD_2 = 12,
  DEVICE_ID_PAD_3 = 13,
  DEVICE_ID_PAD_4 = 14,
  DEVICE_ID_PAD_5 = 15,
  DEVICE_ID_PAD_6 = 16,
  DEVICE_ID_PAD_7 = 17,
  DEVICE_ID_PAD_8 = 18,
  DEVICE_ID_PAD_9 = 19,
  DEVICE_ID_XINPUT_0 = 20,  // XInput joypads
  DEVICE_ID_XINPUT_1 = 21,
  DEVICE_ID_XINPUT_2 = 22,
  DEVICE_ID_XINPUT_3 = 23,
  DEVICE_ID_ACCELEROMETER = 30,  // no longer used
  DEVICE_ID_XR_HMD = 39,
  DEVICE_ID_XR_CONTROLLER_LEFT = 40,
  DEVICE_ID_XR_CONTROLLER_RIGHT = 41,
  DEVICE_ID_TOUCH = 42,
  DEVICE_ID_COUNT,
};
export enum InputAxis {
  // Field descriptor #15 I
  JOYSTICK_AXIS_X = 0,
  JOYSTICK_AXIS_Y = 1,
  JOYSTICK_AXIS_PRESSURE = 2,
  JOYSTICK_AXIS_SIZE = 3,
  JOYSTICK_AXIS_TOUCH_MAJOR = 4,
  JOYSTICK_AXIS_TOUCH_MINOR = 5,
  JOYSTICK_AXIS_TOOL_MAJOR = 6,
  JOYSTICK_AXIS_TOOL_MINOR = 7,
  JOYSTICK_AXIS_ORIENTATION = 8,
  JOYSTICK_AXIS_VSCROLL = 9,
  JOYSTICK_AXIS_HSCROLL = 10,
  JOYSTICK_AXIS_Z = 11,
  JOYSTICK_AXIS_RX = 12,
  JOYSTICK_AXIS_RY = 13,
  JOYSTICK_AXIS_RZ = 14,
  JOYSTICK_AXIS_HAT_X = 15,
  JOYSTICK_AXIS_HAT_Y = 16,
  JOYSTICK_AXIS_LTRIGGER = 17,
  JOYSTICK_AXIS_RTRIGGER = 18,
  JOYSTICK_AXIS_THROTTLE = 19,
  JOYSTICK_AXIS_RUDDER = 20,
  JOYSTICK_AXIS_WHEEL = 21,
  JOYSTICK_AXIS_GAS = 22,
  JOYSTICK_AXIS_BRAKE = 23,
  JOYSTICK_AXIS_DISTANCE = 24,
  JOYSTICK_AXIS_TILT = 25,

  // Ouya axises. Just here for reference, they map straight to regular android axises
  // and will be mapped the same way.
  JOYSTICK_OUYA_AXIS_L2 = 17,
  JOYSTICK_OUYA_AXIS_LS_X = 0,
  JOYSTICK_OUYA_AXIS_LS_Y = 1,
  JOYSTICK_OUYA_AXIS_R2 = 18,
  JOYSTICK_OUYA_AXIS_RS_X = 11,
  JOYSTICK_OUYA_AXIS_RS_Y = 14,

  // Relative mouse axis for PC.
  JOYSTICK_AXIS_MOUSE_REL_X = 26,
  JOYSTICK_AXIS_MOUSE_REL_Y = 27,

  // Mobile device accelerometer/gyro. NOTE: These are no longer passed around internally, only used for the plugin API.
  JOYSTICK_AXIS_ACCELEROMETER_X = 40,
  JOYSTICK_AXIS_ACCELEROMETER_Y = 41,
  JOYSTICK_AXIS_ACCELEROMETER_Z = 42,

  // The numbers must NOT be changed, only additions are allowed
  JOYSTICK_AXIS_MAX = 44
};

export enum VKCode {
  VK_BUTTON_CROSS = 23, // trackpad or X button(Xperia Play) is pressed
  VK_BUTTON_CROSS_PS3 = 96, // PS3 X button is pressed
  VK_BUTTON_CIRCLE = 1004, // Special custom keycode generated from 'O' button by our java code. Or 'O' button if Alt is pressed (TODO)
  VK_BUTTON_CIRCLE_PS3 = 97, // PS3 O button is pressed
  VK_BUTTON_SQUARE = 99, // Square button(Xperia Play) is pressed
  VK_BUTTON_TRIANGLE = 100, // 'Triangle button(Xperia Play) is pressed
  VK_DPAD_UP = 19,
  VK_DPAD_DOWN = 20,
  VK_DPAD_LEFT = 21,
  VK_DPAD_RIGHT = 22,
  VK_DPAD_CENTER = 23,
  VK_UNKNOWN = 0,
  VK_SOFT_LEFT = 1,
  VK_SOFT_RIGHT = 2,
  VK_HOME = 3,
  VK_BACK = 4,
  VK_CALL = 5,
  VK_ENDCALL = 6,
  VK_0 = 7,
  VK_1 = 8,
  VK_2 = 9,
  VK_3 = 10,
  VK_4 = 11,
  VK_5 = 12,
  VK_6 = 13,
  VK_7 = 14,
  VK_8 = 15,
  VK_9 = 16,
  VK_STAR = 17,
  VK_POUND = 18,
  VK_VOLUME_UP = 24,
  VK_VOLUME_DOWN = 25,
  VK_POWER = 26,
  VK_CAMERA = 27,
  VK_CLEAR = 28,
  VK_A = 29,
  VK_B = 30,
  VK_C = 31,
  VK_D = 32,
  VK_E = 33,
  VK_F = 34,
  VK_G = 35,
  VK_H = 36,
  VK_I = 37,
  VK_J = 38,
  VK_K = 39,
  VK_L = 40,
  VK_M = 41,
  VK_N = 42,
  VK_O = 43,
  VK_P = 44,
  VK_Q = 45,
  VK_R = 46,
  VK_S = 47,
  VK_T = 48,
  VK_U = 49,
  VK_V = 50,
  VK_W = 51,
  VK_X = 52,
  VK_Y = 53,
  VK_Z = 54,
  VK_COMMA = 55,
  VK_PERIOD = 56,
  VK_ALT_LEFT = 57,
  VK_ALT_RIGHT = 58,
  VK_SHIFT_LEFT = 59,
  VK_SHIFT_RIGHT = 60,
  VK_TAB = 61,
  VK_SPACE = 62,
  VK_SYM = 63,
  VK_EXPLORER = 64,
  VK_ENVELOPE = 65,
  VK_ENTER = 66,
  VK_DEL = 67,
  VK_GRAVE = 68,
  VK_MINUS = 69,
  VK_EQUALS = 70,
  VK_LEFT_BRACKET = 71,
  VK_RIGHT_BRACKET = 72,
  VK_BACKSLASH = 73,
  VK_SEMICOLON = 74,
  VK_APOSTROPHE = 75,
  VK_SLASH = 76,
  VK_AT = 77,
  VK_NUM = 78,
  VK_HEADSETHOOK = 79,
  VK_FOCUS = 80,
  VK_PLUS = 81,
  VK_MENU = 82,
  VK_NOTIFICATION = 83,
  VK_SEARCH = 84,
  VK_MEDIA_PLAY_PAUSE = 85,
  VK_MEDIA_STOP = 86,
  VK_MEDIA_NEXT = 87,
  VK_MEDIA_PREVIOUS = 88,
  VK_MEDIA_REWIND = 89,
  VK_MEDIA_FAST_FORWARD = 90,
  VK_MUTE = 91,
  VK_PAGE_UP = 92,
  VK_PAGE_DOWN = 93,
  VK_PICTSYMBOLS = 94,
  VK_SWITCH_CHARSET = 95,
  VK_BUTTON_A = 96,
  VK_BUTTON_B = 97,
  VK_BUTTON_C = 98,
  VK_BUTTON_X = 99,
  VK_BUTTON_Y = 100,
  VK_BUTTON_Z = 101,
  VK_BUTTON_L1 = 102,
  VK_BUTTON_R1 = 103,
  VK_BUTTON_L2 = 104,
  VK_BUTTON_R2 = 105,
  VK_BUTTON_THUMBL = 106,
  VK_BUTTON_THUMBR = 107,
  VK_BUTTON_START = 108,
  VK_BUTTON_SELECT = 109,
  VK_BUTTON_MODE = 110,
  VK_ESCAPE = 111,
  VK_FORWARD_DEL = 112,
  VK_CTRL_LEFT = 113,
  VK_CTRL_RIGHT = 114,
  VK_CAPS_LOCK = 115,
  VK_SCROLL_LOCK = 116,
  VK_META_LEFT = 117,
  VK_META_RIGHT = 118,
  VK_FUNCTION = 119,
  VK_SYSRQ = 120,
  VK_BREAK = 121,
  VK_MOVE_HOME = 122,
  VK_MOVE_END = 123,
  VK_INSERT = 124,
  VK_FORWARD = 125,
  VK_MEDIA_PLAY = 126,
  VK_MEDIA_PAUSE = 127,
  VK_MEDIA_CLOSE = 128,
  VK_MEDIA_EJECT = 129,
  VK_MEDIA_RECORD = 130,
  VK_F1 = 131,
  VK_F2 = 132,
  VK_F3 = 133,
  VK_F4 = 134,
  VK_F5 = 135,
  VK_F6 = 136,
  VK_F7 = 137,
  VK_F8 = 138,
  VK_F9 = 139,
  VK_F10 = 140,
  VK_F11 = 141,
  VK_F12 = 142,
  VK_NUM_LOCK = 143,
  VK_NUMPAD_0 = 144,
  VK_NUMPAD_1 = 145,
  VK_NUMPAD_2 = 146,
  VK_NUMPAD_3 = 147,
  VK_NUMPAD_4 = 148,
  VK_NUMPAD_5 = 149,
  VK_NUMPAD_6 = 150,
  VK_NUMPAD_7 = 151,
  VK_NUMPAD_8 = 152,
  VK_NUMPAD_9 = 153,
  VK_NUMPAD_DIVIDE = 154,
  VK_NUMPAD_MULTIPLY = 155,
  VK_NUMPAD_SUBTRACT = 156,
  VK_NUMPAD_ADD = 157,
  VK_NUMPAD_DOT = 158,
  VK_NUMPAD_COMMA = 159,
  VK_NUMPAD_ENTER = 160,
  VK_NUMPAD_EQUALS = 161,
  VK_NUMPAD_LEFT_PAREN = 162,
  VK_NUMPAD_RIGHT_PAREN = 163,
  VK_VOLUME_MUTE = 164,
  VK_INFO = 165,
  VK_CHANNEL_UP = 166,
  VK_CHANNEL_DOWN = 167,
  VK_ZOOM_IN = 168,
  VK_ZOOM_OUT = 169,
  VK_TV = 170,
  VK_WINDOW = 171,
  VK_GUIDE = 172,
  VK_DVR = 173,
  VK_BOOKMARK = 174,
  VK_CAPTIONS = 175,
  VK_SETTINGS = 176,
  VK_TV_POWER = 177,
  VK_TV_INPUT = 178,
  VK_STB_POWER = 179,
  VK_STB_INPUT = 180,
  VK_AVR_POWER = 181,
  VK_AVR_INPUT = 182,
  VK_PROG_RED = 183,
  VK_PROG_GREEN = 184,
  VK_PROG_YELLOW = 185,
  VK_PROG_BLUE = 186,
  VK_APP_SWITCH = 187,
  VK_BUTTON_1 = 188,
  VK_BUTTON_2 = 189,
  VK_BUTTON_3 = 190,
  VK_BUTTON_4 = 191,
  VK_BUTTON_5 = 192,
  VK_BUTTON_6 = 193,
  VK_BUTTON_7 = 194,
  VK_BUTTON_8 = 195,
  VK_BUTTON_9 = 196,
  VK_BUTTON_10 = 197,
  VK_BUTTON_11 = 198,
  VK_BUTTON_12 = 199,
  VK_BUTTON_13 = 200,
  VK_BUTTON_14 = 201,
  VK_BUTTON_15 = 202,
  VK_BUTTON_16 = 203,
  VK_LANGUAGE_SWITCH = 204,
  VK_MANNER_MODE = 205,
  VK_3D_MODE = 206,
  VK_CONTACTS = 207,
  VK_CALENDAR = 208,
  VK_MUSIC = 209,
  VK_CALCULATOR = 210,
  VK_ZENKAKU_HANKAKU = 211,
  VK_EISU = 212,
  VK_MUHENKAN = 213,
  VK_HENKAN = 214,
  VK_KATAKANA_HIRAGANA = 215,
  VK_YEN = 216,
  VK_RO = 217,
  VK_KANA = 218,
  VK_ASSIST = 219,

  // Extended keycodes on latam kayboards.

  VK_START_QUESTION = 220,
  VK_LEFTBRACE = 221,
  VK_RIGHTBRACE = 222,

  // Extended keycodes, not available on Android
  VK_EXT_PIPE = 1001,  // The key next to Z on euro 102-key keyboards.

  VK_EXT_MOUSEBUTTON_1 = 1002,
  VK_EXT_MOUSEBUTTON_2 = 1003,
  VK_EXT_MOUSEBUTTON_3 = 1005,
  VK_EXT_MOUSEBUTTON_4 = 1006,
  VK_EXT_MOUSEBUTTON_5 = 1007,
  VK_EXT_MOUSEWHEEL_UP = 1008,
  VK_EXT_MOUSEWHEEL_DOWN = 1009,

  // Virtual reality motion
  VK_EXT_MOTION_UP = 1101,
  VK_EXT_MOTION_DOWN = 1102,
  VK_EXT_MOTION_LEFT = 1103,
  VK_EXT_MOTION_RIGHT = 1104,
  VK_EXT_MOTION_FORWARD = 1105,

  // Virtual reality rotation
  VK_EXT_ROTATION_UP = 1111,
  VK_EXT_ROTATION_DOWN = 1112,
  VK_EXT_ROTATION_LEFT = 1113,
  VK_EXT_ROTATION_RIGHT = 1114,

  VK_MAX
};

// This is a poor man's mapping between HarmonyOS key codes
export function translateKeyboard(keycode: number): number{
  let translated = 0;
  if (keycode >= KeyCode.KEYCODE_0 && keycode <= KeyCode.KEYCODE_9) {
    translated = (keycode - KeyCode.KEYCODE_0) + VKCode.VK_0;
  }else if (keycode >= KeyCode.KEYCODE_A &&
    keycode <= KeyCode.KEYCODE_Z) {
    translated = (keycode - KeyCode.KEYCODE_A) + VKCode.VK_A;
  }else if (keycode >= KeyCode.KEYCODE_NUMPAD_0 &&
    keycode <= KeyCode.KEYCODE_NUMPAD_9) {
    translated = (keycode - KeyCode.KEYCODE_NUMPAD_0) + VKCode.VK_NUMPAD_0;
  }else if (keycode >= KeyCode.KEYCODE_F1 &&
    keycode <= KeyCode.KEYCODE_F12) {
    translated = (keycode - KeyCode.KEYCODE_F1) + VKCode.VK_F1;
  }else {
    switch (keycode) {
      case KeyCode.KEYCODE_ALT_LEFT:
        translated = VKCode.VK_ALT_LEFT;
        break;
      case KeyCode.KEYCODE_ALT_RIGHT:
        translated = VKCode.VK_ALT_RIGHT;
        break;
      case KeyCode.KEYCODE_BACKSLASH:
        translated = VKCode.VK_BACKSLASH;
        break;
      case KeyCode.KEYCODE_CAPS_LOCK:
        translated = VKCode.VK_CAPS_LOCK;
        break;
      // ohos no
      // case KeyCode.KEYCODE_CLEAR:
      //   translated = VK_CLEAR;
      //   break;

      case KeyCode.KEYCODE_COMMA:
        translated = VKCode.VK_COMMA;
        break;
      case KeyCode.KEYCODE_CTRL_LEFT:
        translated = VKCode.VK_CTRL_LEFT;
        break;
      case KeyCode.KEYCODE_CTRL_RIGHT:
        translated = VKCode.VK_CTRL_RIGHT;
        break;
      case KeyCode.KEYCODE_DEL:
        translated = VKCode.VK_DEL;
        break;
      case KeyCode.KEYCODE_ENTER:
        translated = VKCode.VK_ENTER;
        break;
      case KeyCode.KEYCODE_PLUS:
        translated = VKCode.VK_PLUS;
        break;
      case KeyCode.KEYCODE_EQUALS:
        translated = VKCode.VK_EQUALS;
        break;
      case KeyCode.KEYCODE_ESCAPE:
        translated = VKCode.VK_ESCAPE;
        break;
      case KeyCode.KEYCODE_FORWARD_DEL:
        translated = VKCode.VK_FORWARD_DEL;
        break;
      case KeyCode.KEYCODE_INSERT:
        translated = VKCode.VK_INSERT;
        break;
      case KeyCode.KEYCODE_LEFT_BRACKET:
        translated = VKCode.VK_LEFT_BRACKET;
        break;
      case KeyCode.KEYCODE_META_LEFT:
        translated = VKCode.VK_META_LEFT;
        break;
      case KeyCode.KEYCODE_META_RIGHT:
        translated = VKCode.VK_META_RIGHT;
        break;
      case KeyCode.KEYCODE_MENU:
        translated = VKCode.VK_MENU;
        break;
      case KeyCode.KEYCODE_MINUS:
        translated = VKCode.VK_MINUS;
        break;
      case KeyCode.KEYCODE_MOVE_END:
        translated = VKCode.VK_MOVE_END;
        break;
      case KeyCode.KEYCODE_MOVE_HOME:
        translated = VKCode.VK_HOME;
        break;
      case KeyCode.KEYCODE_NUM_LOCK:
        translated = VKCode.VK_NUM_LOCK;
        break;
      case KeyCode.KEYCODE_PAGE_DOWN:
        translated = VKCode.VK_PAGE_DOWN;
        break;
      case KeyCode.KEYCODE_PAGE_UP:
        translated = VKCode.VK_PAGE_UP;
        break;
      case KeyCode.KEYCODE_PERIOD:
        translated = VKCode.VK_PERIOD
        break;
      case KeyCode.KEYCODE_RIGHT_BRACKET:
        translated = VKCode.VK_RIGHT_BRACKET
        break;
      case KeyCode.KEYCODE_SCROLL_LOCK:
        translated = VKCode.VK_SCROLL_LOCK;
        break;
      case KeyCode.KEYCODE_SEMICOLON:
        translated = VKCode.VK_SEMICOLON
        break;
      case KeyCode.KEYCODE_SHIFT_LEFT:
        translated = VKCode.VK_SHIFT_LEFT
        break;
      case KeyCode.KEYCODE_SHIFT_RIGHT:
        translated = VKCode.VK_SHIFT_RIGHT
        break;
      case KeyCode.KEYCODE_SLASH:
        translated = VKCode.VK_SLASH
        break;
      case KeyCode.KEYCODE_SPACE:
        translated = VKCode.VK_SPACE;
        break;
      case KeyCode.KEYCODE_SYSRQ:
        translated = VKCode.VK_SYSRQ;
        break;
      case KeyCode.KEYCODE_TAB:
        translated = VKCode.VK_TAB;
        break;
      case KeyCode.KEYCODE_DPAD_LEFT:
        translated = VKCode.VK_DPAD_LEFT;
        break;
      case KeyCode.KEYCODE_DPAD_RIGHT:
        translated = VKCode.VK_DPAD_RIGHT;
        break;
      case KeyCode.KEYCODE_DPAD_UP:
        translated = VKCode.VK_DPAD_UP;
        break;
      case KeyCode.KEYCODE_DPAD_DOWN:
        translated = VKCode.VK_DPAD_DOWN;
        break;
      case KeyCode.KEYCODE_GRAVE:
        translated = VKCode.VK_GRAVE;
        break;
      case KeyCode.KEYCODE_APOSTROPHE:
        translated = VKCode.VK_APOSTROPHE;
        break;
      case KeyCode.KEYCODE_BREAK:
        translated = VKCode.VK_BREAK;
        break;
      case KeyCode.KEYCODE_NUMPAD_DIVIDE:
        translated = VKCode.VK_NUMPAD_DIVIDE
        break;
      case KeyCode.KEYCODE_NUMPAD_MULTIPLY:
        translated = VKCode.VK_NUMPAD_MULTIPLY
        break;
      case KeyCode.KEYCODE_NUMPAD_SUBTRACT:
        translated = VKCode.VK_NUMPAD_SUBTRACT
        break;
      case KeyCode.KEYCODE_NUMPAD_ADD:
        translated = VKCode.VK_NUMPAD_ADD
        break;
      case KeyCode.KEYCODE_NUMPAD_DOT:
        translated = VKCode.VK_NUMPAD_DOT
        break;
      default:
        return 0;
    }
  }
  return translated;
}


export function translateController(keycode: number): number{
    switch (keycode){
      case KeyCode.KEYCODE_BUTTON_THUMBL : return VKCode.VK_BUTTON_THUMBL
      case KeyCode.KEYCODE_BUTTON_THUMBR : return VKCode.VK_BUTTON_THUMBR
      case KeyCode.KEYCODE_BUTTON_Y : return VKCode.VK_BUTTON_Y
      case KeyCode.KEYCODE_BUTTON_X : return VKCode.VK_BUTTON_X
      case KeyCode.KEYCODE_BUTTON_B : return VKCode.VK_BUTTON_B
      case KeyCode.KEYCODE_BUTTON_A : return VKCode.VK_BUTTON_A
      case KeyCode.KEYCODE_DPAD_UP : return VKCode.VK_DPAD_UP
      case KeyCode.KEYCODE_DPAD_LEFT : return VKCode.VK_DPAD_LEFT
      case KeyCode.KEYCODE_DPAD_RIGHT : return VKCode.VK_DPAD_RIGHT
      case KeyCode.KEYCODE_DPAD_DOWN : return VKCode.VK_DPAD_DOWN
      case KeyCode.KEYCODE_BUTTON_SELECT : return VKCode.VK_BUTTON_SELECT
      case KeyCode.KEYCODE_BUTTON_MODE : return VKCode.VK_BUTTON_MODE
      case KeyCode.KEYCODE_BUTTON_START : return VKCode.VK_BUTTON_START
      case KeyCode.KEYCODE_BUTTON_L2 : return VKCode.VK_BUTTON_L2
      case KeyCode.KEYCODE_BUTTON_L1 : return VKCode.VK_BUTTON_L1
      case KeyCode.KEYCODE_BUTTON_R2 : return VKCode.VK_BUTTON_R2
      case KeyCode.KEYCODE_BUTTON_R1 : return VKCode.VK_BUTTON_R1
    }
    return 0;
}

export function translateAxis(keycode: number): number {
  switch (keycode){
    case AxisModel.ABS_X : return InputAxis.JOYSTICK_AXIS_X
    case AxisModel.ABS_Y : return InputAxis.JOYSTICK_AXIS_Y
    case AxisModel.ABS_Z : return InputAxis.JOYSTICK_AXIS_Z
    case AxisModel.ABS_RZ : return InputAxis.JOYSTICK_AXIS_RZ
    case AxisModel.ABS_GAS : return InputAxis.JOYSTICK_AXIS_GAS
    case AxisModel.ABS_BRAKE : return InputAxis.JOYSTICK_AXIS_BRAKE
    case AxisModel.ABS_HAT0X : return InputAxis.JOYSTICK_AXIS_HAT_X
    case AxisModel.ABS_HAT0Y : return InputAxis.JOYSTICK_AXIS_HAT_Y
  }
  return 0;
}
