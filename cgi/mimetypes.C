/*
 * mimetypes.C: Translate file extensions into http document types.
 *
 * Version: $Revision: 0.1 $
 * Author: Benjamin Oshrin
 * Date: $Date: 2004/09/09 12:58:28 $
 * MT-Level: Safe
 *
 * Copyright (c) 2004
 * The Trustees of Columbia University in the City of New York
 * Academic Information Systems
 * 
 * License restrictions apply, see doc/license.html for details.
 *
 * $Log: mimetypes.C,v $
 * Revision 0.1  2004/09/09 12:58:28  benno
 * Initial Revision
 *
 */

#include "cgi.H"

char *mime_ext_to_type(char *ext)
{
  // Translate <ext> (eg: "html") to a document type (eg: "text/html").

  // Returns: A string that should not be deleted or modified, or NULL
  // if no translation is found.

  // We could do this with a cf or even an external cf such as apache's,
  // but this simplifies initial configuration (at the expense of adding
  // new types easily).  This list comes from Apache's mime.types file.

  char *mtypes[] = {
    "ez", "application/andrew-inset",
    "hqx", "application/mac-binhex40",
    "cpt", "application/mac-compactpro",
    "doc", "application/msword",
    "bin", "application/octet-stream",
    "dms", "application/octet-stream",
    "lha", "application/octet-stream",
    "lzh", "application/octet-stream",
    "exe", "application/octet-stream",
    "class", "application/octet-stream",
    "so", "application/octet-stream",
    "dll", "application/octet-stream",
    "oda", "application/oda",
    "pdf", "application/pdf",
    "ai", "application/postscript",
    "eps", "application/postscript",
    "ps", "application/postscript",
    "smi", "application/smil",
    "smil", "application/smil",
    "mif", "application/vnd.mif",
    "xls", "application/vnd.ms-excel",
    "ppt", "application/vnd.ms-powerpoint",
    "wbxml", "application/vnd.wap.wbxml",
    "wmlc", "application/vnd.wap.wmlc",
    "wmlsc", "application/vnd.wap.wmlscriptc",
    "bcpio", "application/x-bcpio",
    "vcd", "application/x-cdlink",
    "pgn", "application/x-chess-pgn",
    "cpio", "application/x-cpio",
    "csh", "application/x-csh",
    "dcr", "application/x-director",
    "dir", "application/x-director",
    "dxr", "application/x-director",
    "dvi", "application/x-dvi",
    "spl", "application/x-futuresplash",
    "gtar", "application/x-gtar",
    "hdf", "application/x-hdf",
    "js", "application/x-javascript",
    "skp", "application/x-koan",
    "skd", "application/x-koan",
    "skt", "application/x-koan",
    "skm", "application/x-koan",
    "latex", "application/x-latex",
    "nc", "application/x-netcdf",
    "cdf", "application/x-netcdf",
    "sh", "application/x-sh",
    "shar", "application/x-shar",
    "swf", "application/x-shockwave-flash",
    "sit", "application/x-stuffit",
    "sv4cpio", "application/x-sv4cpio",
    "sv4crc", "application/x-sv4crc",
    "tar", "application/x-tar",
    "tcl", "application/x-tcl",
    "tex", "application/x-tex",
    "texinfo", "application/x-texinfo",
    "texi", "application/x-texinfo",
    "t", "application/x-troff",
    "tr", "application/x-troff",
    "roff", "application/x-troff",
    "man", "application/x-troff-man",
    "me", "application/x-troff-me",
    "ms", "application/x-troff-ms",
    "ustar", "application/x-ustar",
    "src", "application/x-wais-source",
    "xhtml", "application/xhtml+xml",
    "xht", "application/xhtml+xml",
    "zip", "application/zip",
    "au", "audio/basic",
    "snd", "audio/basic",
    "mid", "audio/midi",
    "midi", "audio/midi",
    "kar", "audio/midi",
    "mpga", "audio/mpeg",
    "mp2", "audio/mpeg",
    "mp3", "audio/mpeg",
    "aif", "audio/x-aiff",
    "aiff", "audio/x-aiff",
    "aifc", "audio/x-aiff",
    "m3u", "audio/x-mpegurl",
    "ram", "audio/x-pn-realaudio",
    "rm", "audio/x-pn-realaudio",
    "rpm", "audio/x-pn-realaudio-plugin",
    "ra", "audio/x-realaudio",
    "wav", "audio/x-wav",
    "pdb", "chemical/x-pdb",
    "xyz", "chemical/x-xyz",
    "bmp", "image/bmp",
    "gif", "image/gif",
    "ief", "image/ief",
    "jpeg", "image/jpeg",
    "jpg", "image/jpeg",
    "jpe", "image/jpeg",
    "png", "image/png",
    "tiff", "image/tiff",
    "tif", "image/tiff",
    "djvu", "image/vnd.djvu",
    "djv", "image/vnd.djvu",
    "wbmp", "image/vnd.wap.wbmp",
    "ras", "image/x-cmu-raster",
    "pnm", "image/x-portable-anymap",
    "pbm", "image/x-portable-bitmap",
    "pgm", "image/x-portable-graymap",
    "ppm", "image/x-portable-pixmap",
    "rgb", "image/x-rgb",
    "xbm", "image/x-xbitmap",
    "xpm", "image/x-xpixmap",
    "xwd", "image/x-xwindowdump",
    "igs", "model/iges",
    "iges", "model/iges",
    "msh", "model/mesh",
    "mesh", "model/mesh",
    "silo", "model/mesh",
    "wrl", "model/vrml",
    "vrml", "model/vrml",
    "css", "text/css",
    "html", "text/html",
    "htm", "text/html",
    "asc", "text/plain",
    "txt", "text/plain",
    "rtx", "text/richtext",
    "rtf", "text/rtf",
    "sgml", "text/sgml",
    "sgm", "text/sgml",
    "tsv", "text/tab-separated-values",
    "wml", "text/vnd.wap.wml",
    "wmls", "text/vnd.wap.wmlscript",
    "etx", "text/x-setext",
    "xml", "text/xml",
    "xsl", "text/xml",
    "mpeg", "video/mpeg",
    "mpg", "video/mpeg",
    "mpe", "video/mpeg",
    "qt", "video/quicktime",
    "mov", "video/quicktime",
    "mxu", "video/vnd.mpegurl",
    "avi", "video/x-msvideo",
    "movie", "video/x-sgi-movie",
    "ice", "x-conference/x-cooltalk",
    NULL, NULL
  };
  
  if(ext)
  {
    for(int i = 0;mtypes[i] != NULL;i+=2)
    {
      if(strcmp(ext, mtypes[i])==0)
	return(mtypes[i+1]);
    }
  }
  
  return(NULL);
}
