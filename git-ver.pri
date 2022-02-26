QT += core network
QT += gui
QT += widgets
QT += serialport


GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD/ describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
#VERSION = $$GIT_VERSION
#    win32 {
#        VERSION ~= s/-\d+-g[a-f0-9]{6,}//
#    }

##DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" describe)\\\""
#GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
#GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --abbrev-ref HEAD)\\\""
##BUILD_TIMESTAMP="\\\"$$system(date -u +\""%Y-%m-%dT%H:%M:%SUTC\"")\\\""
#DEFINES += GIT_HASH=$$GIT_HASH GIT_BRANCH=$$GIT_BRANCH #BUILD_TIMESTAMP=$$BUILD_TIMESTAMP
