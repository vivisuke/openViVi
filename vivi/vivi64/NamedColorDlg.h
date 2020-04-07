#ifndef NAMEDCOLORDLG_H
#define NAMEDCOLORDLG_H

/*

	Copyright (C) 2012 by Nobuhide Tsuda

	RuviEdit �̃��C�Z���X�� MIT�{��GPL �ȃ��C�Z���X�ł��B 
	���ۏ؁E���T�|�[�g�ł����A�����ŗ��p�ł��A���p�A�v���ł��\�[�X�R�[�h�𗬗p���邱�Ƃ��\�ł��B 
	�i�\�[�X�R�[�h�𗬗p�����ꍇ�A���p�����̒��쌠�E���C�Z���X��RuviEdit�̂���̂܂܂ł��j 
	�������M�҂́A�v���O���}�ɂƂ��ĕs���R�ɂ܂�Ȃ��̂Ɏ��R�����R���ƌ�������GPL�n�������Ȃ̂ŁA 
	RuviEdit �̃\�[�X��GPL�n�v���W�F�N�g�Ŏg�p���邱�Ƃ��֎~���܂��B 
	GPL�v���W�F�N�g�ł͈�؂̗��p���֎~���܂����ALGPL�v���W�F�N�g�ł͓��I�����N�ɂ�闬�p�͋����܂��B

*/

#include <QDialog>

class NamedColorDlg : public QDialog
{
	Q_OBJECT

public:
	NamedColorDlg(QWidget *parent = 0);
	~NamedColorDlg();

public:
	QString colorName() const { return m_colorName; }

public slots:
	void	accept();

protected slots:
	void	setupColorList();
	void	itemDoubleClicked ( class QListWidgetItem * item );

private:
	QString	m_colorName;
	class QLineEdit *m_lineEdit;
	class QListWidget *m_listWidget;
};

#endif // NAMEDCOLORDLG_H
