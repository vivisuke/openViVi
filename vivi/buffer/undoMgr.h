//----------------------------------------------------------------------
//
//			File:			"undoMgr.h"
//			Created:		16-Jun-2013
//			Author:			�Óc�L�G
//			Description:
//
//----------------------------------------------------------------------

#pragma once

#ifndef		_HEADER_UNDOMGR_H
#define		_HEADER_UNDOMGR_H

#define		USE_BOOST_POOL		0

#include <vector>
#if		USE_BOOST_POOL
#include <boost/pool/object_pool.hpp>
#else
#define		POOL_SIZE		1024
#endif

class Buffer;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef 

/*
	�ЂƂ̃A�N�V������ UndoAction �I�u�W�F�N�g�ŕ\�����
	��A�̃A�N�V�������܂Ƃ߂� undo/redo �������ꍇ�́AUndoAction::m_flags �� FLAG_BLOCK �r�b�g��ON�ɂ��Ă���
		UndoAction(flags: 0);
		UndoAction(flags: 1);		��������
		UndoAction(flags: 0);
		:
		UndoAction(flags: 1);		�����܂ł��ЂƂ̃u���b�N�ƂȂ�
		UndoAction(flags: 0);
*/
struct UndoAction
{
	enum {
		ACT_UNDEF = 0,
		ACT_INSERT,
		ACT_DELETE,
		ACT_REPLACE,
		//ACT_REPLACE_ALL,

		FLAG_BLOCK = 1,		//	���̃r�b�g�������Ă���΁Aundo block �J�n or �I��
		//FLAG_MODIFIED = 2,	//	���f�B�t�@�C�t���O
		FLAG_BS = 2,		//	BackSpace �ɂ��폜
	};
	uchar	m_type;
	uchar	m_flags;

public:
	UndoAction(uchar type = 0, uchar flags = 0)
		: m_type(type)
		, m_flags(flags)
		{}
#if 0
	UndoAction(int type = 0, int pos = 0, int size = 0, int ix = 0)
		: m_type(type)
		, m_flags(0)
		, m_pos(pos)
		, m_size(size)
		, m_ix(ix)
		{}
#endif
	virtual ~UndoAction() {};
};
struct UndoActionInsert : public UndoAction
{
	int		m_pos;		//	�}���E�폜�ʒu
	int		m_size;		//	�}���E�폜�������ifor UTF16�j
	int		m_ix;		//	�}���E�폜�������Undo�o�b�t�@�C���f�b�N�X
	int		m_ln;		//	pos �̍s�ԍ� 0 org
	int		m_lc;		//	�}���s��
	bool	m_modified;	//	�}���O�� m_ln �s���ҏW�ς݂��������ǂ���
	bool	m_saved;	//	�}���O�� m_ln �s���ۑ��ς݂��������ǂ���
	bool	m_prohibitMerge;	//	�}�[�W�֎~
	//std::vector<bool>	m_mdfyFlags;
public:
	UndoActionInsert(int pos = 0, int size = 0 /*, uchar flags = 0*/)
		: UndoAction(ACT_INSERT /*, flags*/)
		, m_pos(pos)
		, m_size(size)
		, m_ix(0)
		, m_prohibitMerge(false)
		{}
	~UndoActionInsert() {}
};
struct UndoActionDelete : public UndoAction
{
	int		m_pos;		//	�}���E�폜�ʒu
	int		m_size;		//	�}���E�폜�������ifor UTF16�j
	int		m_ix;		//	�}���E�폜�������Undo�o�b�t�@�C���f�b�N�X
	int		m_ln;		//	pos �̍s�ԍ� 0 org
	int		m_lc;		//	�폜�s��
	//int		m_mdfyIX;	//	�s���ۑ��ʒu
	//std::vector<bool>	m_mdfyFlags;
public:
	UndoActionDelete(int pos = 0, int size = 0, int ix = 0, bool BS = false)
		: UndoAction(ACT_DELETE, (BS ? FLAG_BS : 0))
		, m_pos(pos)
		, m_size(size)
		, m_ix(ix)
		{}
	~UndoActionDelete() {}
};
struct UndoActionReplace : public UndoAction
{
	int		m_pos;		//	�}���E�폜�ʒu
	int		m_ln;		//	pos �̍s�ԍ� 0 org
	int		m_sizeIns;	//	�}���������ifor UTF16�j
	int		m_ixIns;	//	�}���������Undo�o�b�t�@�C���f�b�N�X
	int		m_lcIns;	//	�}���s��
	int		m_sizeDel;	//	�폜�������ifor UTF16�j
	int		m_ixDel;	//	�폜�������Undo�o�b�t�@�C���f�b�N�X
	int		m_lcDel;	//	�폜�s��
public:
	UndoActionReplace(int pos = 0, int ln = 0,
						int sizeIns = 0, int ixIns = 0, int lcIns = 0,
						int sizeDel = 0, int ixDel = 0, int lcDel = 0)
		: UndoAction(ACT_REPLACE /*, flags*/)
		, m_pos(pos)
		, m_ln(ln)
		, m_sizeIns(sizeIns)
		, m_ixIns(ixIns)
		, m_sizeDel(sizeDel)
		, m_ixDel(ixDel)
		, m_lcDel(lcDel)
		{}
	~UndoActionReplace() {}
};
#if 0
struct UndoActionReplaceAll : public UndoAction
{
	int		*m_posList;		//	�}���E�폜�ʒu
	int		m_nReplace;			//	�u����
	const wchar_t	*m_before;
	int		m_beforeSz;	//	�폜�������ifor UTF16�j
	const wchar_t	*m_after;
	int		m_afterSz;	//	�}���������ifor UTF16�j
	//std::vector<bool>	m_mdfyFlags;
public:
	UndoActionReplaceAll(int *posList, int nReplace,
						const wchar_t *before, int beforeSz,
						const wchar_t *after, int afterSz /*,uchar flags = 0*/)
		: UndoAction(ACT_REPLACE_ALL /*, flags*/)
		, m_posList(posList)
		, m_nReplace(nReplace)
		, m_before(before)
		, m_beforeSz(beforeSz)
		, m_after(after)
		, m_afterSz(afterSz)
		{}
	~UndoActionReplaceAll()
	{
		delete m_posList;
		delete m_before;
		delete m_after;
	}
};
#endif
//template<typename T>
class UndoMgr
{
	//typedef wchar_t char_t;

public:
	UndoMgr(Buffer *);
	~UndoMgr();

public:
	bool	canUndo() const { return m_cur != 0; }
	bool	canRedo() const { return (size_t)m_cur < m_stack.size(); }
	int		undoIndex() const { return m_cur; }
	bool	isBlockOpened() const { return m_blockLevel != 0; }
	//bool	isBlockOpened() const { return m_blockOpened; }

public:
	void	init();
	int		undo();
	int		redo();
	bool	push_back(UndoAction *);
	bool	push_back_delText(int, int, bool BS, int ln);
	bool	push_back_insText(int, int, int ln);
	UndoActionReplace	*push_back_repText(int, int dsz, int isz, int ln);
	void	setLcIns(int);
	void	openBlock();
	void	closeBlock();
	void	closeAllBlock();
	//void	resetModified();	//	����Ԃ�񃂃f�B�t�@�C��ԂƂ���i�ۑ����ɃR�[�������j
	void	onSaved();			//	����Ԃ�񃂃f�B�t�@�C��ԂƂ���i�ۑ����ɃR�[�������j
	void	prohibitMergeUndo();			//	�}���}�[�W�֎~

protected:
#if !USE_BOOST_POOL
	UndoActionInsert	*newActInsert();
	UndoActionDelete	*newActDelete();
	UndoActionReplace	*newActReplace();
#endif

//private:
public:
	Buffer	*m_buffer;		//	�ҏW�o�b�t�@
	int		m_cur;			//	m_stack ���݈ʒu
	int		m_savePointCur;		//	���f�B�t�@�C��Ԃ̏ꏊ�i�ۑ��|�C���g�j
	int		m_lastDelTextSize;		//	�Ō�̍폜������
	int		m_lastInsTextSize;
	int		m_blockLevel;
	//bool	m_blockOpened;
	bool	m_actionPushed;			//	�I�[�v�����ɃA�N�V�������ǉ����ꂽ
	std::vector<UndoAction *>	m_stack;
	std::vector<wchar_t>		m_delText;		//	�폜������
	std::vector<wchar_t>		m_insText;		//	�}��������

#if USE_BOOST_POOL
	boost::object_pool<UndoActionInsert>	m_actInsPool;
	boost::object_pool<UndoActionDelete>	m_actDelPool;
	boost::object_pool<UndoActionReplace>	m_actRepPool;
#else
	int		m_actInsPoolSize;
	int		m_actDelPoolSize;
	int		m_actRepPoolSize;
	std::vector<UndoActionInsert *>	m_actInsPool;
	std::vector<UndoActionDelete *>	m_actDelPool;
	std::vector<UndoActionReplace *>	m_actRepPool;
#endif

friend class TestClass;
};

#endif		//_HEADER_UNDOMGR_H
