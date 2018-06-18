
basePath = $${dirname(PWD)}
for(header, INSTALL_HEADERS) {
  relPath = $${relative_path($$header, $$basePath)}
  path = $${INSTALL_PREFIX}/$${dirname(relPath)}
  eval(headers_$${path}.files += $$relPath)
  eval(headers_$${path}.path = $$path)
  eval(INSTALLS *= headers_$${path})
}

