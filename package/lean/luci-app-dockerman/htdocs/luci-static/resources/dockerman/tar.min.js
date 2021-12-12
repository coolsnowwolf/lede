// https://github.com/thiscouldbebetter/TarFileExplorer
class TarFileTypeFlag
{constructor(value,name)
{this.value=value;this.id="_"+this.value;this.name=name;}
static _instances;static Instances()
{if(TarFileTypeFlag._instances==null)
{TarFileTypeFlag._instances=new TarFileTypeFlag_Instances();}
return TarFileTypeFlag._instances;}}
class TarFileTypeFlag_Instances
{constructor()
{this.Normal=new TarFileTypeFlag("0","Normal");this.HardLink=new TarFileTypeFlag("1","Hard Link");this.SymbolicLink=new TarFileTypeFlag("2","Symbolic Link");this.CharacterSpecial=new TarFileTypeFlag("3","Character Special");this.BlockSpecial=new TarFileTypeFlag("4","Block Special");this.Directory=new TarFileTypeFlag("5","Directory");this.FIFO=new TarFileTypeFlag("6","FIFO");this.ContiguousFile=new TarFileTypeFlag("7","Contiguous File");this.LongFilePath=new TarFileTypeFlag("L","././@LongLink");this._All=[this.Normal,this.HardLink,this.SymbolicLink,this.CharacterSpecial,this.BlockSpecial,this.Directory,this.FIFO,this.ContiguousFile,this.LongFilePath,];for(var i=0;i<this._All.length;i++)
{var item=this._All[i];this._All[item.id]=item;}}}
class TarFileEntryHeader
{constructor
(fileName,fileMode,userIDOfOwner,userIDOfGroup,fileSizeInBytes,timeModifiedInUnixFormat,checksum,typeFlag,nameOfLinkedFile,uStarIndicator,uStarVersion,userNameOfOwner,groupNameOfOwner,deviceNumberMajor,deviceNumberMinor,filenamePrefix)
{this.fileName=fileName;this.fileMode=fileMode;this.userIDOfOwner=userIDOfOwner;this.userIDOfGroup=userIDOfGroup;this.fileSizeInBytes=fileSizeInBytes;this.timeModifiedInUnixFormat=timeModifiedInUnixFormat;this.checksum=checksum;this.typeFlag=typeFlag;this.nameOfLinkedFile=nameOfLinkedFile;this.uStarIndicator=uStarIndicator;this.uStarVersion=uStarVersion;this.userNameOfOwner=userNameOfOwner;this.groupNameOfOwner=groupNameOfOwner;this.deviceNumberMajor=deviceNumberMajor;this.deviceNumberMinor=deviceNumberMinor;this.filenamePrefix=filenamePrefix;}
static FileNameMaxLength=99;static SizeInBytes=500;static default()
{var now=new Date();var unixEpoch=new Date(1970,1,1);var millisecondsSinceUnixEpoch=now-unixEpoch;var secondsSinceUnixEpoch=Math.floor
(millisecondsSinceUnixEpoch/1000);var secondsSinceUnixEpochAsStringOctal=secondsSinceUnixEpoch.toString(8).padRight(12,"\0");var timeModifiedInUnixFormat=[];for(var i=0;i<secondsSinceUnixEpochAsStringOctal.length;i++)
{var digitAsASCIICode=secondsSinceUnixEpochAsStringOctal.charCodeAt(i);timeModifiedInUnixFormat.push(digitAsASCIICode);}
var returnValue=new TarFileEntryHeader
("".padRight(100,"\0"),"0100777","0000000","0000000",0,timeModifiedInUnixFormat,0,TarFileTypeFlag.Instances().Normal,"","ustar","00","","","","","");return returnValue;};static directoryNew(directoryName)
{var header=TarFileEntryHeader.default();header.fileName=directoryName;header.typeFlag=TarFileTypeFlag.Instances().Directory;header.fileSizeInBytes=0;header.checksumCalculate();return header;};static fileNew(fileName,fileContentsAsBytes)
{var header=TarFileEntryHeader.default();header.fileName=fileName;header.typeFlag=TarFileTypeFlag.Instances().Normal;header.fileSizeInBytes=fileContentsAsBytes.length;header.checksumCalculate();return header;};static fromBytes(bytes)
{var reader=new ByteStream(bytes);var fileName=reader.readString(100).trim();var fileMode=reader.readString(8);var userIDOfOwner=reader.readString(8);var userIDOfGroup=reader.readString(8);var fileSizeInBytesAsStringOctal=reader.readString(12);var timeModifiedInUnixFormat=reader.readBytes(12);var checksumAsStringOctal=reader.readString(8);var typeFlagValue=reader.readString(1);var nameOfLinkedFile=reader.readString(100);var uStarIndicator=reader.readString(6);var uStarVersion=reader.readString(2);var userNameOfOwner=reader.readString(32);var groupNameOfOwner=reader.readString(32);var deviceNumberMajor=reader.readString(8);var deviceNumberMinor=reader.readString(8);var filenamePrefix=reader.readString(155);var reserved=reader.readBytes(12);var fileSizeInBytes=parseInt
(fileSizeInBytesAsStringOctal.trim(),8);var checksum=parseInt
(checksumAsStringOctal,8);var typeFlags=TarFileTypeFlag.Instances()._All;var typeFlagID="_"+typeFlagValue;var typeFlag=typeFlags[typeFlagID];var returnValue=new TarFileEntryHeader
(fileName,fileMode,userIDOfOwner,userIDOfGroup,fileSizeInBytes,timeModifiedInUnixFormat,checksum,typeFlag,nameOfLinkedFile,uStarIndicator,uStarVersion,userNameOfOwner,groupNameOfOwner,deviceNumberMajor,deviceNumberMinor,filenamePrefix);return returnValue;};checksumCalculate()
{var thisAsBytes=this.toBytes();var offsetOfChecksumInBytes=148;var numberOfBytesInChecksum=8;var presumedValueOfEachChecksumByte=" ".charCodeAt(0);for(var i=0;i<numberOfBytesInChecksum;i++)
{var offsetOfByte=offsetOfChecksumInBytes+i;thisAsBytes[offsetOfByte]=presumedValueOfEachChecksumByte;}
var checksumSoFar=0;for(var i=0;i<thisAsBytes.length;i++)
{var byteToAdd=thisAsBytes[i];checksumSoFar+=byteToAdd;}
this.checksum=checksumSoFar;return this.checksum;};toBytes()
{var headerAsBytes=[];var writer=new ByteStream(headerAsBytes);var fileSizeInBytesAsStringOctal=(this.fileSizeInBytes.toString(8)+"\0").padLeft(12,"0")
var checksumAsStringOctal=(this.checksum.toString(8)+"\0 ").padLeft(8,"0");writer.writeString(this.fileName,100);writer.writeString(this.fileMode,8);writer.writeString(this.userIDOfOwner,8);writer.writeString(this.userIDOfGroup,8);writer.writeString(fileSizeInBytesAsStringOctal,12);writer.writeBytes(this.timeModifiedInUnixFormat);writer.writeString(checksumAsStringOctal,8);writer.writeString(this.typeFlag.value,1);writer.writeString(this.nameOfLinkedFile,100);writer.writeString(this.uStarIndicator,6);writer.writeString(this.uStarVersion,2);writer.writeString(this.userNameOfOwner,32);writer.writeString(this.groupNameOfOwner,32);writer.writeString(this.deviceNumberMajor,8);writer.writeString(this.deviceNumberMinor,8);writer.writeString(this.filenamePrefix,155);writer.writeString("".padRight(12,"\0"));return headerAsBytes;};toString()
{var newline="\n";var returnValue="[TarFileEntryHeader "
+"fileName='"+this.fileName+"' "
+"typeFlag='"+(this.typeFlag==null?"err":this.typeFlag.name)+"' "
+"fileSizeInBytes='"+this.fileSizeInBytes+"' "
+"]"
+newline;return returnValue;};}
class TarFileEntry
{constructor(header,dataAsBytes)
{this.header=header;this.dataAsBytes=dataAsBytes;}
static directoryNew(directoryName)
{var header=TarFileEntryHeader.directoryNew(directoryName);var entry=new TarFileEntry(header,[]);return entry;};static fileNew(fileName,fileContentsAsBytes)
{var header=TarFileEntryHeader.fileNew(fileName,fileContentsAsBytes);var entry=new TarFileEntry(header,fileContentsAsBytes);return entry;};static fromBytes(chunkAsBytes,reader)
{var chunkSize=TarFile.ChunkSize;var header=TarFileEntryHeader.fromBytes
(chunkAsBytes);var sizeOfDataEntryInBytesUnpadded=header.fileSizeInBytes;var numberOfChunksOccupiedByDataEntry=Math.ceil
(sizeOfDataEntryInBytesUnpadded/chunkSize)
var sizeOfDataEntryInBytesPadded=numberOfChunksOccupiedByDataEntry*chunkSize;var dataAsBytes=reader.readBytes
(sizeOfDataEntryInBytesPadded).slice
(0,sizeOfDataEntryInBytesUnpadded);var entry=new TarFileEntry(header,dataAsBytes);return entry;};static manyFromByteArrays
(fileNamePrefix,fileNameSuffix,entriesAsByteArrays)
{var returnValues=[];for(var i=0;i<entriesAsByteArrays.length;i++)
{var entryAsBytes=entriesAsByteArrays[i];var entry=TarFileEntry.fileNew
(fileNamePrefix+i+fileNameSuffix,entryAsBytes);returnValues.push(entry);}
return returnValues;};download(event)
{FileHelper.saveBytesAsFile
(this.dataAsBytes,this.header.fileName);};remove(event)
{alert("Not yet implemented!");};toBytes()
{var entryAsBytes=[];var chunkSize=TarFile.ChunkSize;var headerAsBytes=this.header.toBytes();entryAsBytes=entryAsBytes.concat(headerAsBytes);entryAsBytes=entryAsBytes.concat(this.dataAsBytes);var sizeOfDataEntryInBytesUnpadded=this.header.fileSizeInBytes;var numberOfChunksOccupiedByDataEntry=Math.ceil
(sizeOfDataEntryInBytesUnpadded/chunkSize)
var sizeOfDataEntryInBytesPadded=numberOfChunksOccupiedByDataEntry*chunkSize;var numberOfBytesOfPadding=sizeOfDataEntryInBytesPadded-sizeOfDataEntryInBytesUnpadded;for(var i=0;i<numberOfBytesOfPadding;i++)
{entryAsBytes.push(0);}
return entryAsBytes;};toString()
{var newline="\n";headerAsString=this.header.toString();var dataAsHexadecimalString=ByteHelper.bytesToStringHexadecimal
(this.dataAsBytes);var returnValue="[TarFileEntry]"+newline
+headerAsString
+"[Data]"
+dataAsHexadecimalString
+"[/Data]"+newline
+"[/TarFileEntry]"
+newline;return returnValue}}
class TarFile
{constructor(fileName,entries)
{this.fileName=fileName;this.entries=entries;}
static ChunkSize=512;static fromBytes(fileName,bytes)
{var reader=new ByteStream(bytes);var entries=[];var chunkSize=TarFile.ChunkSize;var numberOfConsecutiveZeroChunks=0;while(reader.hasMoreBytes()==true)
{var chunkAsBytes=reader.readBytes(chunkSize);var areAllBytesInChunkZeroes=true;for(var b=0;b<chunkAsBytes.length;b++)
{if(chunkAsBytes[b]!=0)
{areAllBytesInChunkZeroes=false;break;}}
if(areAllBytesInChunkZeroes==true)
{numberOfConsecutiveZeroChunks++;if(numberOfConsecutiveZeroChunks==2)
{break;}}
else
{numberOfConsecutiveZeroChunks=0;var entry=TarFileEntry.fromBytes(chunkAsBytes,reader);entries.push(entry);}}
var returnValue=new TarFile(fileName,entries);returnValue.consolidateLongPathEntries();return returnValue;}
static create(fileName)
{return new TarFile
(fileName,[]);}
consolidateLongPathEntries()
{var typeFlagLongPathName=TarFileTypeFlag.Instances().LongFilePath.name;var entries=this.entries;for(var i=0;i<entries.length;i++)
{var entry=entries[i];if(entry.header.typeFlag.name==typeFlagLongPathName)
{var entryNext=entries[i+1];entryNext.header.fileName=entry.dataAsBytes.reduce
((a,b)=>a+=String.fromCharCode(b),"");entryNext.header.fileName=entryNext.header.fileName.replace(/\0/g,"");entries.splice(i,1);i--;}}}
downloadAs(fileNameToSaveAs)
{return FileHelper.saveBytesAsFile
(this.toBytes(),fileNameToSaveAs)}
entriesForDirectories()
{return this.entries.filter(x=>x.header.typeFlag.name==TarFileTypeFlag.Instances().Directory);}
toBytes()
{this.toBytes_PrependLongPathEntriesAsNeeded();var fileAsBytes=[];var entriesAsByteArrays=this.entries.map(x=>x.toBytes());this.consolidateLongPathEntries();for(var i=0;i<entriesAsByteArrays.length;i++)
{var entryAsBytes=entriesAsByteArrays[i];fileAsBytes=fileAsBytes.concat(entryAsBytes);}
var chunkSize=TarFile.ChunkSize;var numberOfZeroChunksToWrite=2;for(var i=0;i<numberOfZeroChunksToWrite;i++)
{for(var b=0;b<chunkSize;b++)
{fileAsBytes.push(0);}}
return fileAsBytes;}
toBytes_PrependLongPathEntriesAsNeeded()
{var typeFlagLongPath=TarFileTypeFlag.Instances().LongFilePath;var maxLength=TarFileEntryHeader.FileNameMaxLength;var entries=this.entries;for(var i=0;i<entries.length;i++)
{var entry=entries[i];var entryHeader=entry.header;var entryFileName=entryHeader.fileName;if(entryFileName.length>maxLength)
{var entryFileNameAsBytes=entryFileName.split("").map(x=>x.charCodeAt(0));var entryContainingLongPathToPrepend=TarFileEntry.fileNew
(typeFlagLongPath.name,entryFileNameAsBytes);entryContainingLongPathToPrepend.header.typeFlag=typeFlagLongPath;entryContainingLongPathToPrepend.header.timeModifiedInUnixFormat=entryHeader.timeModifiedInUnixFormat;entryContainingLongPathToPrepend.header.checksumCalculate();entryHeader.fileName=entryFileName.substr(0,maxLength)+String.fromCharCode(0);entries.splice(i,0,entryContainingLongPathToPrepend);i++;}}}
toString()
{var newline="\n";var returnValue="[TarFile]"+newline;for(var i=0;i<this.entries.length;i++)
{var entry=this.entries[i];var entryAsString=entry.toString();returnValue+=entryAsString;}
returnValue+="[/TarFile]"+newline;return returnValue;}}
function StringExtensions()
{}
{String.prototype.padLeft=function(lengthToPadTo,charToPadWith)
{var returnValue=this;while(returnValue.length<lengthToPadTo)
{returnValue=charToPadWith+returnValue;}
return returnValue;}
String.prototype.padRight=function(lengthToPadTo,charToPadWith)
{var returnValue=this;while(returnValue.length<lengthToPadTo)
{returnValue+=charToPadWith;}
return returnValue;}}
class Globals
{static Instance=new Globals();}
class FileHelper
{static loadFileAsBytes(fileToLoad,callback)
{var fileReader=new FileReader();fileReader.onload=(fileLoadedEvent)=>{var fileLoadedAsBinaryString=fileLoadedEvent.target.result;var fileLoadedAsBytes=ByteHelper.stringUTF8ToBytes(fileLoadedAsBinaryString);callback(fileToLoad.name,fileLoadedAsBytes);}
fileReader.readAsBinaryString(fileToLoad);}
static loadFileAsText(fileToLoad,callback)
{var fileReader=new FileReader();fileReader.onload=(fileLoadedEvent)=>{var textFromFileLoaded=fileLoadedEvent.target.result;callback(fileToLoad.name,textFromFileLoaded);};fileReader.readAsText(fileToLoad);}
static saveBytesAsFile(bytesToWrite,fileNameToSaveAs)
{var bytesToWriteAsArrayBuffer=new ArrayBuffer(bytesToWrite.length);var bytesToWriteAsUIntArray=new Uint8Array(bytesToWriteAsArrayBuffer);for(var i=0;i<bytesToWrite.length;i++)
{bytesToWriteAsUIntArray[i]=bytesToWrite[i];}
var bytesToWriteAsBlob=new Blob
([bytesToWriteAsArrayBuffer],{type:"application/type"});
return bytesToWriteAsBlob
// var downloadLink=document.createElement("a");downloadLink.download=fileNameToSaveAs;downloadLink.href=window.URL.createObjectURL(bytesToWriteAsBlob);downloadLink.click();
}
static saveTextAsFile(textToSave,fileNameToSaveAs)
{var textToSaveAsBlob=new Blob([textToSave],{type:"text/plain"});var textToSaveAsURL=window.URL.createObjectURL(textToSaveAsBlob);var downloadLink=document.createElement("a");downloadLink.download=fileNameToSaveAs;downloadLink.href=textToSaveAsURL;downloadLink.click();}}
class ByteStream
{constructor(bytes)
{this.bytes=bytes;this.byteIndexCurrent=0;}
static BitsPerByte=8;static BitsPerByteTimesTwo=ByteStream.BitsPerByte*2;static BitsPerByteTimesThree=ByteStream.BitsPerByte*3;hasMoreBytes()
{return(this.byteIndexCurrent<this.bytes.length);}
readBytes(numberOfBytesToRead)
{var returnValue=new Array(numberOfBytesToRead);for(var b=0;b<numberOfBytesToRead;b++)
{returnValue[b]=this.readByte();}
return returnValue;}
readByte()
{var returnValue=this.bytes[this.byteIndexCurrent];this.byteIndexCurrent++;return returnValue;}
readString(lengthOfString)
{var returnValue="";for(var i=0;i<lengthOfString;i++)
{var byte=this.readByte();if(byte!=0)
{var byteAsChar=String.fromCharCode(byte);returnValue+=byteAsChar;}}
return returnValue;}
writeBytes(bytesToWrite)
{for(var b=0;b<bytesToWrite.length;b++)
{this.bytes.push(bytesToWrite[b]);}
this.byteIndexCurrent=this.bytes.length;}
writeByte(byteToWrite)
{this.bytes.push(byteToWrite);this.byteIndexCurrent++;}
writeString(stringToWrite,lengthPadded)
{for(var i=0;i<stringToWrite.length;i++)
{var charAsByte=stringToWrite.charCodeAt(i);this.writeByte(charAsByte);}
var numberOfPaddingChars=lengthPadded-stringToWrite.length;for(var i=0;i<numberOfPaddingChars;i++)
{this.writeByte(0);}}}
class ByteHelper
{static stringUTF8ToBytes(stringToConvert)
{var bytes=[];for(var i=0;i<stringToConvert.length;i++)
{var byte=stringToConvert.charCodeAt(i);bytes.push(byte);}
return bytes;}
static bytesToStringUTF8(bytesToConvert)
{var returnValue="";for(var i=0;i<bytesToConvert.length;i++)
{var byte=bytesToConvert[i];var byteAsChar=String.fromCharCode(byte);returnValue+=byteAsChar}
return returnValue;}}
function ArrayExtensions()
{}
{Array.prototype.remove=function(elementToRemove)
{this.splice(this.indexOf(elementToRemove),1);}}