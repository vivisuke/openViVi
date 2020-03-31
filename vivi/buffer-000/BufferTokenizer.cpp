//----------------------------------------------------------------------
//
//			File:			"BufferTokenizer.cpp"
//			Created:		29-9-2013
//			Author:			í√ìcêLèG
//			Description:
//
//----------------------------------------------------------------------

#include "BufferTokenizer.h"
#include "Buffer.h"

BufferTokenizer::BufferTokenizer(const Buffer &buffer, int pos, int last)
	: m_buffer(buffer)
	, m_pos(pos)
	, m_last(last)
{
}
