#!/usr/bin/perl

# Internal utility to take doc/*.in and perform the appropriate substitutions
#
# Usage: docbuilder <srcdir> <destdir>

if(scalar(@ARGV) != 3) { die("Usage: $0 <srcdir> <destdir> <pkgversion>"); }

$srcdir = $ARGV[0];
$destdir = $ARGV[1];
$pkgvers = $ARGV[2];

$buildf = $srcdir . "/building.html.in";
$indexf = $srcdir . "/index.html.in";
$licensef = $srcdir . "/license.html.in";

# Build a list of keywords from .html files.  Note this won't get
# .html.in files, but that's OK for now since none of them have keywords
# (If this becomes an issue, try iterating through dest instead of src.)

# Also look for dependencies while we're here

$keywords = "";
%deps;

if(opendir(SRCDIR, $srcdir)) {
    @files = sort(grep(/.html$/, readdir(SRCDIR)));
    closedir(SRCDIR);

    foreach $f (@files) {
	next if ($f eq "cm-template.html");  # skip the template
	
	printf("Parsing $f...\n");

	if(open(IN, $srcdir."/".$f))
	{
	    $titledone = 0;
	    
	    while(<IN>)
	    {
		if(/CLASS="keywords"/) {
		    /CLASS=\"keywords\" HREF=\"?([^\">]*)\"?>([^<]*)<\/A>/;
		    $href = $f.$1;
		    $txt = $2;

		    if(!$titledone)
		    {
			$keywords .= "$f<BR>\n";
			$titledone++;
		    }

		    $keywords .= "<A CLASS=\"keywords\" HREF=\"" . $href
			. "\">" . $txt . "</A><BR>\n";
		}
		elsif(/<SURVIVOR META=\"dependency\"/)
		{
		    /TYPE=\"([^\"]+)\"/;
		    $dtype = $1;

		    $x = $f;
		    $x =~ s/\.html//;
		    ($mtype, $mname) = split(/-/, $x);

		    $txt = "";

		    while(<IN>)
		    {
			if(/<\/SURVIVOR/)
			{
			    last;
			}
			else
			{
			    $txt .= $_;
			}
		    }
		    
		    # eg: $deps{"cm"}{"foo"}{"perl"}
		    $deps{$mtype}{$mname}{$dtype} = $txt;
		}
	    }
	    
	    close(IN);
	}
	else
	{
	    printf("unable to open file");
	}
    }
}

if(-r $indexf) {
    $indexf2 = $destdir . "/index.html";
    
    print "Processing $indexf to $indexf2\n";

    if(open(IN, $indexf))
    {
	if(open(OUT, "> $indexf2"))
	{
	    while(<IN>) {
		$_ =~ s/\@PKGVERSION\@/$pkgvers/;
		$_ =~ s/\@KEYWORDS\@/$keywords/;

		print OUT $_;
	    }
	    
	    close(OUT);
	}
	else
	{
	    print "Error opening $indexf2\n";
	}

	close(IN);
    }
    else
    {
	print "Error opening $indexf\n";
    }
}

if(-r $licensef) {
    $licensef2 = $destdir . "/license.html";
    
    print "Processing $licensef to $licensef2\n";

    if(open(IN, $licensef))
    {
	if(open(OUT, "> $licensef2"))
	{
	    while(<IN>) {
		$_ =~ s/\@PKGVERSION\@/$pkgvers/;

		print OUT $_;
	    }
	    
	    close(OUT);
	}
	else
	{
	    print "Error opening $licensef2\n";
	}

	close(IN);
    }
    else
    {
	print "Error opening $licensef\n";
    }
}

if(-r $buildf) {
    $buildf2 = $destdir . "/building.html";
    
    print "Processing $buildf to $buildf2\n";

    if(open(IN, $buildf))
    {
	if(open(OUT, "> $buildf2"))
	{
	    while(<IN>) {
		if(/\@CHECKDEPENDENCIES\@/)
		{
		    $mode = "cm";
		}
		elsif(/\@TRANSMITDEPENDENCIES\@/)
		{
		    $mode = "tmm";
		}
		elsif(/\@WEBAUTHDEPENDENCIES\@/)
		{
		    $mode = "wam";
		}
		else
		{
		    $mode = "";
		}

		if($mode ne "")
		{
		    @keys = sort(keys %{$deps{$mode}});
		    
		    foreach $k (@keys)
		    {
			print OUT "<TR>\n";
			print OUT "<TH CLASS=\"default\"><A HREF=\"" .
			    $mode . "-" . $k . ".html\">" . $k . "</A></TH>\n";
			print OUT "<TD CLASS=\"default\">\n";
			
			if(defined($deps{$mode}{$k}{"perl"})) {
			    print OUT $deps{$mode}{$k}{"perl"};
			} else {
			    print OUT "-\n";
			}

			print OUT "</TD>\n";
			print OUT "<TD CLASS=\"default\">\n";
			
			if(defined($deps{$mode}{$k}{"executable"})) {
			    print OUT $deps{$mode}{$k}{"executable"};
			} else {
			    print OUT "-\n";
			}

			print OUT "</TD>\n";
			print OUT "</TR>\n\n";
		    }
		}
		else
		{
		    print OUT $_;
		}
	    }
	    
	    close(OUT);
	}
	else
	{
	    print "Error opening $buildf2\n";
	}

	close(IN);
    }
    else
    {
	print "Error opening $buildf\n";
    }
}

exit(0);
