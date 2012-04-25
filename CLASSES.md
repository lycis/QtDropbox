# QtDropbox Classes

This is the first - very short - approach to something that one day will be a 
complete class documentation. This document lists all classes that are bundled 
in QtDropbox and describes their purpose and most important methods. 

## QDropbox
### Description
Provides the _main entry point_ for QtDropbox. You use this class to connect to a 
Dropbox account and to access other features like account information.

### Prominent methods
#### void requestToken()
Initiates the oAuth sign in process. This method starts the authentication and results 
in a temporary token. First to be called for not yet authorized connections.

#### void requestAccessToken()
Call this method if you already have a token (does not matter if that is from requestToken() 
or an already authorized token). After this method is called the Dropbox connection
is usually ready for use. Use this function if you already have an authorized token and
secret from.

#### QDropboxAccount accountInfo()
Requests and returns account information from Dropbox.


## QDropboxAccount
### Description
This class provides information related to the connected acoount (equivalent to the
/account/info REST API call).

### Prominent methods
I refrain mentioning them here. There is one method to get each of the account
information entries listed in the [API description](https://www.dropbox.com/developers/reference/api)

## QDropboxFile
### Description
Whenever you want to access a file stored on Dropbox use this class. It behaves
like you may know from QFile and provides read and write access (currently read only).
This class is designed to work woth QTextStream or QDataStream - just as you would
expect it.

### Prominent methods
#### qint64 readData(char *data, qint64 maxlen)
Receives data from a file in the Dropbox. The file content is requested and stored in
a local buffer. This function is usually not called directly but from any of the
Q<Something>Stream classes.

#### qint64 writeData(const char *data, qint64 len)
Writes to a file in the Dropbox. Automatically flushes if the internal buffer reaches
a configured threshold (1024 bytes of new data by default).


## QDropboxFileInfo
### Description
This class contains metadata of files or directories in the Dropbox. (not implemented)

## QDropboxJson
### Description
Actually this class is mostly designed for internal use. It is used to
interpret the JSONs that are replied by the REST API. You may use QDropboxJson
for your own purposes of course.

### Prominent methods
#### void parseString(QString strJson)
Interprets a string and fills the JSON with data from that string.

#### Accessing functions
There are a lot of functions to access a value from the JSON. One for every possible
datatype: int, unsigned int, string, double, bool and finally there may be a JSON
embedded in another JSON.
