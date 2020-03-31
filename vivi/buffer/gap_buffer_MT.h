//----------------------------------------------------------------------
//
//			File:			"gap_buffer_MT.h"
//			Created:		16-Mar-2020
//			Author:			津田伸秀
//			Description:
//				マルチスレッドを用いてデータ移動を行うギャップバッファ
//
//----------------------------------------------------------------------

#pragma once

#include "gap_buffer.h"

template<typename Type>
class gap_buffer_MT : public gap_buffer<Type>
{
};

