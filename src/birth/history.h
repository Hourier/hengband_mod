﻿
/*!
* 生い立ちメッセージテーブル / Forward declare
*/
typedef struct hist_type hist_type;

/*!
* 生い立ちメッセージテーブルの構造体定義 / Player background information
*/
struct hist_type
{
	concptr info;			    /*!> メッセージ本文 / Textual History */

	byte roll;			    /*!> 確率の重み / Frequency of this entry */
	byte chart;			    /*!> 生い立ちメッセージの流れを示すチャートID / Chart index */
	byte next;			    /*!> 次のチャートID */
	byte bonus;			    /*!> メッセージに伴う社会的地位の変化量(50が基準値) / Social Class Bonus + 50 */
};

struct hist_type;
extern hist_type bg[];
