/*

	Copyright (C) 2012 by Nobuhide Tsuda

*/
#include <QtGui>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <QDebug>
#include "NamedColorDlg.h"

typedef const char cchar;

struct SColorTable {
	uint	m_color;
    cchar	*m_name;
};

static SColorTable colorTable[] = {
	{0xfff8f0, "aliceblue"}, {0xd7ebfa, "antiquewhite"}, {0xffff00, "aqua"}, {0xd4ff7f, "aquamarine"},
	{0xfffff0, "azure"}, {0xdcf5f5, "beige"}, {0xc4e4ff, "bisque"}, {0x000000, "black"},
	{0xcdebff, "blanchedalmond"}, {0xff0000, "blue"}, {0xe22b8a, "blueviolet"}, {0x2a2aa5, "brown"},
	{0x87b8de, "burlywood"}, {0xa09e5f, "cadetblue"}, {0x00ff7f, "chartreuse"}, {0x1e69d2, "chocolate"},
	{0x507fff, "coral"}, {0xed9564, "cornflowerblue"}, {0xdcf8ff, "cornsilk"}, {0x3c14dc, "crimson"},
	{0xffff00, "cyan"}, {0x8b0000, "darkblue"}, {0x8b8b00, "darkcyan"}, {0x0b86b8, "darkgoldenrod"},
	{0xa9a9a9, "darkgray"}, {0x006400, "darkgreen"}, {0x6bb7bd, "darkkhaki"}, {0x8b008b, "darkmagenta"},
	{0x2f6b55, "darkolivegreen"}, {0x008cff, "darkorange"}, {0xcc3299, "darkorchid"}, {0x00008b, "darkred"},
	{0x7a96e9, "darksalmon"}, {0x8fbc8f, "darkseagreen"}, {0x8b3d48, "darkslateblue"}, {0x4f4f2f, "darkslategray"},
	{0xd1ce00, "darkturquoise"}, {0xd30094, "darkviolet"}, {0x9314ff, "deeppink"}, {0xffbf00, "deepskyblue"},
	{0x696969, "dimgray"}, {0xff901e, "dodgerblue"}, {0x2222b2, "firebrick"}, {0xf0faff, "floralwhite"},
	{0x228b22, "forestgreen"}, {0xff00ff, "fuchsia"}, {0xdcdcdc, "gainsboro"}, {0xfff8f8, "ghostwhite"},
	{0x00d7ff, "gold"}, {0x20a5da, "goldenrod"}, {0x808080, "gray"}, {0x008000, "green"},
	{0x2fffad, "greenyellow"}, {0xf0fff0, "honeydew"}, {0xb469ff, "hotpink"}, {0x5c5ccd, "indianred"},
	{0x82004b, "indigo"}, {0xf0ffff, "ivory"}, {0x8ce6f0, "khaki"}, {0xfae6e6, "lavender"},
	{0xf5f0ff, "lavenderblush"}, {0x00fc7c, "lawngreen"}, {0xcdfaff, "lemonchiffon"}, {0xe6d8ad, "lightblue"},
	{0x8080f0, "lightcoral"}, {0xffffe0, "lightcyan"}, {0xd2fafa, "lightgoldenrodyellow"}, {0x90ee90, "lightgreen"},
	{0xd3d3d3, "lightgrey"}, {0xc1b6ff, "lightpink"}, {0x7aa0ff, "lightsalmon"}, {0xaab220, "lightseagreen"},
	{0xface87, "lightskyblue"}, {0x998877, "lightslategray"}, {0xdec4b0, "lightsteelblue"}, {0xe0ffff, "lightyellow"},
	{0x00ff00, "lime"}, {0x32cd32, "limegreen"}, {0xe6f0fa, "linen"}, {0xff00ff, "magenta"},
	{0x000080, "maroon"}, {0xaacd66, "mediumaquamarine"}, {0xcd0000, "mediumblue"}, {0xd355ba, "mediumorchid"},
	{0xdb7093, "mediumpurple"}, {0x71b33c, "mediumseagreen"}, {0xee687b, "mediumslateblue"}, {0x9afa00, "mediumspringgreen"},
	{0xccd148, "mediumturquoise"}, {0x8515c7, "mediumvioletred"}, {0x701919, "midnightblue"}, {0xfafff5, "mintcream"},
	{0xe1e4ff, "mistyrose"}, {0xb5e4ff, "moccasin"}, {0xaddeff, "navajowhite"}, {0x800000, "navy"},
	{0xe6f5fd, "oldlace"}, {0x008080, "olive"}, {0x238e6b, "olivedrab"}, {0x00a5ff, "orange"},
	{0x0045ff, "orangered"}, {0xd670da, "orchid"}, {0xaae8ee, "palegoldenrod"}, {0x98fb98, "palegreen"},
	{0xeeeeaf, "paleturquoise"}, {0x9370db, "palevioletred"}, {0xd5efff, "papayawhip"}, {0xb9daff, "peachpuff"},
	{0x3f85cd, "peru"}, {0xcbc0ff, "pink"}, {0xdda0dd, "plum"}, {0xe6e0b0, "powderblue"},
	{0x800080, "purple"}, {0x0000ff, "red"}, {0x8f8fbc, "rosybrown"}, {0xe16941, "royalblue"},
	{0x13458b, "saddlebrown"}, {0x7280fa, "salmon"}, {0x60a4f4, "sandybrown"}, {0x578b2e, "seagreen"},
	{0xeef5ff, "seashell"}, {0x2d52a0, "sienna"}, {0xc0c0c0, "silver"}, {0xebce87, "skyblue"},
	{0xcd5a6a, "slateblue"}, {0x908070, "slategray"}, {0xfafaff, "snow"}, {0x7fff00, "springgreen"},
	{0xb48246, "steelblue"}, {0x8cb4d2, "tan"}, {0x808000, "teal"}, {0xd8bfd8, "thistle"},
	{0x4763ff, "tomato"}, {0xd0e040, "turquoise"}, {0xee82ee, "violet"}, {0xb3def5, "wheat"},
	{0xffffff, "white"}, {0xf5f5f5, "whitesmoke"}, {0x00ffff, "yellow"}, {0x32cd9a, "yellowgreen"},
#if 0
	{0x01000000, "sysColor0"}, {0x01000001, "sysColor1"}, {0x01000002, "sysColor2"}, {0x01000003, "sysColor3"},
	{0x01000004, "sysColor4"}, {0x01000005, "sysColor5"}, {0x01000006, "sysColor6"}, {0x01000007, "sysColor7"},
	{0x01000008, "sysColor8"}, {0x01000009, "sysColor9"}, {0x0100000a, "sysColor10"}, {0x0100000b, "sysColor11"},
	{0x0100000c, "sysColor12"}, {0x0100000d, "sysColor13"}, {0x0100000e, "sysColor14"}, {0x0100000f, "sysColor15"},
	{0x01000010, "sysColor16"}, {0x01000011, "sysColor17"}, {0x01000012, "sysColor18"}, {0x01000013, "sysColor19"},
#endif
	{0, 0}
};

NamedColorDlg::NamedColorDlg(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout;
		QPushButton *okButton = new QPushButton(tr("OK"));
		okButton->setDefault(true);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		QHBoxLayout *hLayout = new QHBoxLayout;
			hLayout->addStretch();
			hLayout->addWidget(okButton);
		m_lineEdit = new QLineEdit;
		connect(m_lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setupColorList()));
		m_listWidget = new QListWidget;
		setupColorList();
		connect(m_listWidget, SIGNAL(itemDoubleClicked ( QListWidgetItem *)),
				this, SLOT(itemDoubleClicked ( QListWidgetItem *)));
		layout->addWidget(m_lineEdit);
		layout->addWidget(m_listWidget);
		layout->addLayout(hLayout);
	setLayout(layout);
}

NamedColorDlg::~NamedColorDlg()
{

}

void NamedColorDlg::accept()
{
	QListWidgetItem *item = m_listWidget->currentItem();
	if( item == 0 ) {
		qDebug() << "???\n";
		return;
	}
	m_colorName = item->text();
	QDialog::accept();
}

void NamedColorDlg::itemDoubleClicked ( QListWidgetItem * item )
{
	m_colorName = item->text();
	QDialog::accept();
}

QIcon createColorIcon(int color)
{
    QPixmap pixmap(20, 20);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    const int r = color & 0xff;
    const int g = (color>>8) & 0xff;
    const int b = (color>>16) & 0xff;
    painter.fillRect(QRect(0, 0, 20, 20), QColor(r, g, b));
    return QIcon(pixmap);
}
void NamedColorDlg::setupColorList()
{
	for(int ix = m_listWidget->count() - 1; ix >= 0; --ix) {
		QListWidgetItem *ptr = m_listWidget->takeItem(ix);
		delete ptr;
	}
	QString fillter = m_lineEdit->text();
	QListWidgetItem *curItem = 0;
	for(SColorTable *ptr = colorTable; ptr->m_name != 0; ++ptr) {
		if( fillter.isEmpty() || QString(ptr->m_name).startsWith(fillter) ) {
			QListWidgetItem *item = new QListWidgetItem(createColorIcon(ptr->m_color), ptr->m_name, m_listWidget);
			if( curItem == 0 )
				m_listWidget->setCurrentItem(curItem = item);
		}
	}
}

