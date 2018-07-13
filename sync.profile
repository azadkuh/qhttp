%modules = (
    "QtHttp" => "$basedir/src/http",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%mastercontent = (
    "core" => "#include <QtCore/QtCore>",
    "network" => "#include <QtNetwork/QtNetwork>"
);
%modulepris = (
    "QtHttp" => "$basedir/modules/qt_http.pri",
);

%dependencies = (
        "qtbase" => ""
);
