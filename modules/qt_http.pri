QT.http.VERSION = 5.0.0
QT.http.MAJOR_VERSION = 5
QT.http.MINOR_VERSION = 0
QT.http.PATCH_VERSION = 0

QT.http.name = QtHttp
QT.http.bins = $$QT_MODULE_BIN_BASE
QT.http.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtHttp
QT.http.private_includes = $$QT_MODULE_INCLUDE_BASE/QtHttp/$$QT.http.VERSION
QT.http.sources = $$QT_MODULE_BASE/src/http
QT.http.libs = $$QT_MODULE_LIB_BASE
QT.http.plugins = $$QT_MODULE_PLUGIN_BASE
QT.http.imports = $$QT_MODULE_IMPORT_BASE
QT.http.depends = core network
