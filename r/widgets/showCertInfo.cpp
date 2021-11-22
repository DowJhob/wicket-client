#include "showCertInfo.h"

showCertInfo::showCertInfo()
{
    setFixedWidth( 480 );
    setFixedHeight( 640 );
    setLayout( &background_layout );
    pixmap.load(":images/service.jpg");
    palette.setBrush(QPalette::Background, pixmap);
    setPalette(palette);
}
