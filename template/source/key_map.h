#ifndef _H_KEY_MAP_
#define _H_KEY_MAP_

// #include <gba.h>

// //INPUT情報
// // #define REG_KEYINPUT	*(vu16*)(REG_BASE + 0x130)  // Key Input

// // typedef enum KEYPAD_BITS {
// // 	KEY_A		=	(1<<0),	/*!< keypad A button */
// // 	KEY_B		=	(1<<1),	/*!< keypad B button */
// // 	KEY_SELECT	=	(1<<2),	/*!< keypad SELECT button */
// // 	KEY_START	=	(1<<3),	/*!< keypad START button */
// // 	KEY_RIGHT	=	(1<<4),	/*!< dpad RIGHT */
// // 	KEY_LEFT	=	(1<<5),	/*!< dpad LEFT */
// // 	KEY_UP		=	(1<<6),	/*!< dpad UP */
// // 	KEY_DOWN	=	(1<<7),	/*!< dpad DOWN */
// // 	KEY_R		=	(1<<8),	/*!< Right shoulder button */
// // 	KEY_L		=	(1<<9),	/*!< Left shoulder button */

// // 	KEYIRQ_ENABLE	=	(1<<14),	/*!< Enable keypad interrupt */
// // 	KEYIRQ_OR	=	(0<<15),	/*!< interrupt logical OR mode */
// // 	KEYIRQ_AND	=	(1<<15),	/*!< interrupt logical AND mode */
// // 	DPAD 		=	(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT) /*!< mask all dpad buttons */
// // } KEYPAD_BITS;

// ボタンの状態を保持する構造体
// typedef struct {
//     u16 key;  // 現在のボタンの状態
//     u16 keyr; // 長押し状態
//     u16 key_up; // 上げ判定
//     u16 keysHeld; // ボタンが押され続けている時間（フレーム数）
// } KeyState;

// KeyState keyState;

// // ボタンの状態を更新する関数
// void updateKeyState() {
//     scanKeys();
//     keyState.key = keysDown();  // 1フレーム前の状態を保持
//     keyState.keyr = keysDownRepeat(); // 現在のボタンの状態を読み取る
//     keyState.key_up = keysUp();
//     keyState.keysHeld = keysHeld();
// }

#endif	// _H_KEY_MAP_
