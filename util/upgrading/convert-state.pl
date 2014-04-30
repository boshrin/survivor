#!/usr/bin/perl

# Script to convert from v0.9.4 state format to v0.9.5 format.
# The scheduler must not be running while this script is run.
#
# Usage: convert-state.pl <statedir>

# Version: $Revision: 0.2 $
# Author: Benjamin Oshrin
# Date: $Date: 2004/10/04 21:17:04 $

use FindBin;
use lib "$FindBin::Bin/../../modules/common";
use Survivor::XML;

# We do two things, toss the entire state/service tree and convert the
# state/host tree

$svcstatedir = $ARGV[0] . "/service";

# Even though we're removing the whole tree, we still use service/state
# to store tmpstatus files.  Fortunately, the scheduler will recreate
# the tree when it is restarted after this conversion.

printf("Removing service state tree...");
system("rm -fr $svcstatedir");

# Now convert the state/host tree

printf("Beginning state conversion...\n");
$statedir = $ARGV[0] . "/host";

if(opendir(STATEDIR, $statedir)) {
    @hosts = grep(!/^\.\.?/, readdir(STATEDIR));
    closedir(STATEDIR);

    $done = 0;
    
    foreach $h (@hosts) {
	$hostdir = $statedir . "/" . $h;

	printf("Processing $h (%d%% done)\n",
	       (($done * 100) / scalar(@hosts)));

	if(opendir(HOSTDIR, $hostdir)) {
	    @services = grep(!/^\.\.?/, readdir(HOSTDIR));
	    closedir(HOSTDIR);

	    foreach $s (@services) {
		# We look for alertstatus, checkstatus, and fixstatus,
		# and convert whatever we find.  We also look for
		# acknowledge and noalert (inhibit records) and escalations.

		@todo = ("alertstatus", "checkstatus", "fixstatus",
			 "acknowledge", "noalert", "escalate");

		foreach $t (@todo) {
		    $hfile = $hostdir . "/" . $s . "/" . $t;
		    $newfile = $hfile . ".new";

		    $ok = 0;
		    $skip = 0;  # If previously converted
		    
		    if(open(INFILE, $hfile)) {
			@statinfo = stat(INFILE);

			if(open(OUTFILE, "> $newfile")) {
			    # Copy and convert the contents from the old file
			    # to the new

			    my $sx = new Survivor::XML();

			    # Everything starts the same
			    my %xmltree = ("name" => "xml",
					   "text" => "",
					   "attrs" => {},
					   "children" => {},
					   "parent" => undef);

			    if($t eq "alertstatus") {
				# Old format is
				#  lastalert
                                #  check return code
				#  consecutive
				#  0 or more lines of the form
			        #    rc:module:address

				my %adata = ("name" => "SurvivorAlertStatus",
					     "text" => "",
					     "attrs" => {"Version" => "1.0"},
					     "children" => {},
					     "childorder" => ["RecipientSet",
							      "Instances",
							      "CheckReturnCode",
							      "CheckSummary",
							      "Time"],
					     "parent" => \%xmltree);
				$xmltree{"children"}{"SurvivorAlertStatus"} =
				    [\%adata];

				$tm = <INFILE>;
				chomp($tm);
				$crc = <INFILE>;
				chomp($crc);
				$con = <INFILE>;
				chomp($con);

				if($tm =~ /xml/) {
				    # We've seen this one already
				    $skip++;
				}
				
				my %atdata = ("name" => "Time",
					      "text" => $tm,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%adata);
				$adata{"children"}{"Time"} = [\%atdata];

				my %acrcdata = ("name" => "CheckReturnCode",
						"text" => $crc,
						"attrs" => {},
						"children" => {},
						"parent" => \%adata);
				$adata{"children"}{"CheckReturnCode"} =
				    [\%acrcdata];

				my %aidata = ("name" => "Instances",
					      "text" => $con,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%adata);
				$adata{"children"}{"Instances"} = [\%aidata];

				my %arsdata = ("name" => "RecipientSet",
					       "text" => "",
					       "attrs" => {},
					       "children" => {},
					       "childorder" => ["Module"],
					       "parent" => \%adata);
				$adata{"children"}{"RecipientSet"} =
				    [\%arsdata];
				
				while(<INFILE>) {
				    # Iterate through each recipient line

				    my $x = $_;
				    chomp($x);
				    
				    my ($a_rc, $a_mod, $a_addr) =
					split(/:/, $x);

				    my %amdata = ("name" => "Module",
						  "text" => "",
						  "attrs" =>
						    {"Name" => $a_mod},
						  "children" => {},
						  "childorder" =>
						    ["Address", "ReturnCode"],
						  "parent" => \%arsdata);
				    push(@{$arsdata{"children"}{"Module"}},
					 \%amdata);

				    my %amadata = ("name" => "Address",
						   "text" => $a_addr,
						   "attrs" => {},
						   "children" => {},
						   "parent" => \%amdata);
				    $amdata{"children"}{"Address"} =
					[\%amadata];

				    my %amrcdata = ("name" => "ReturnCode",
						    "text" => $a_rc,
						    "attrs" => {},
						    "children" => {},
						    "parent" => \%amdata);
				    $amdata{"children"}{"ReturnCode"} =
					[\%amrcdata];
				}
			    } elsif($t eq "checkstatus") {
				# Old format is
				#  lastcheck
				#  returncode
				#  consecutive
				#  comment

				my %cdata = ("name" => "SurvivorCheckStatus",
					     "text" => "",
					     "attrs" => {"Version" => "1.0"},
					     "children" => {},
					     "childorder" => ["Instances",
							      "ReturnCode",
							      "Summary",
							      "Time"],
					     "parent" => \%xmltree);
				$xmltree{"children"}{"SurvivorCheckStatus"} =
				    [\%cdata];

				$tm = <INFILE>;
				chomp($tm);
				$rc = <INFILE>;
				chomp($rc);
				$con = <INFILE>;
				chomp($con);
				$cmt = <INFILE>;
				chomp($cmt);

				if($tm =~ /xml/) {
				    # We've seen this one already
				    $skip++;
				}

				my %ctdata = ("name" => "Time",
					      "text" => $tm,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%cdata);
				$cdata{"children"}{"Time"} = [\%ctdata];
				
				my %crcdata = ("name" => "ReturnCode",
					       "text" => $rc,
					       "attrs" => {},
					       "children" => {},
					       "parent" => \%cdata);
				$cdata{"children"}{"ReturnCode"} = [\%crcdata];

				my %ccdata = ("name" => "Instances",
					      "text" => $con,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%cdata);
				$cdata{"children"}{"Instances"} = [\%ccdata];

				my %csdata = ("name" => "Summary",
					      "text" => $cmt,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%cdata);
				$cdata{"children"}{"Summary"} = [\%csdata];
			    } elsif($t eq "fixstatus") {
				# Old format is
				#  lastfix
				#  number of fixes
				#  return code
				#  who
				#  comment
				
				my %fdata = ("name" => "SurvivorFixStatus",
					     "text" => "",
					     "attrs" => {"Version" => "1.0"},
					     "children" => {},
					     "childorder" => ["InitiatedBy",
							      "Instances",
							      "ReturnCode",
							      "Summary",
							      "Time"],
					     "parent" => \%xmltree);
				$xmltree{"children"}{"SurvivorFixStatus"} =
				    [\%fdata];

				$tm = <INFILE>;
				chomp($tm);
				$con = <INFILE>;
				chomp($con);
				$rc = <INFILE>;
				chomp($rc);
				$by = <INFILE>;
				chomp($by);
				$cmt = <INFILE>;
				chomp($cmt);

				if($tm =~ /xml/) {
				    # We've seen this one already
				    $skip++;
				}
	
				my %ftdata = ("name" => "Time",
					      "text" => $tm,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%fdata);
				$fdata{"children"}{"Time"} = [\%ftdata];

				my %fcdata = ("name" => "Instances",
					      "text" => $con,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%fdata);
				$fdata{"children"}{"Instances"} = [\%fcdata];

				my %frcdata = ("name" => "ReturnCode",
					       "text" => $rc,
					       "attrs" => {},
					       "children" => {},
					       "parent" => \%fdata);
				$fdata{"children"}{"ReturnCode"} = [\%frcdata];

				my %fwdata = ("name" => "InitiatedBy",
					      "text" => $by,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%fdata);
				$fdata{"children"}{"InitiatedBy"} = [\%fwdata];

				my %fsdata = ("name" => "Summary",
					      "text" => $cmt,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%fdata);
				$fdata{"children"}{"Summary"} = [\%fsdata];
			    } elsif($t eq "acknowledge" || $t eq "noalert") {
				# Old format is
				#  who
				#  why

				if($t eq "acknowledge") {
				    $xmlelement = "SurvivorAlertAcknowledgement";
				} else {
				    $xmlelement = "SurvivorAlertInhibition";
				}
				
				my %adata = ("name" => $xmlelement,
					     "text" => "",
					     "attrs" => {"Version" => "1.0"},
					     "children" => {},
					     "childorder" => ["Who",
							      "Why"],
					     "parent" => \%xmltree);
				$xmltree{"children"}{$xmlelement} =
				    [\%adata];

				$who = <INFILE>;
				chomp($who);
				$why = <INFILE>;
				chomp($why);
				
				if($who =~ /xml/) {
				    # We've seen this one already
				    $skip++;
				}
				
				my %aodata = ("name" => "Who",
					      "text" => $who,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%adata);
				$adata{"children"}{"Who"} = [\%aodata];

				my %aydata = ("name" => "Why",
					      "text" => $why,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%adata);
				$adata{"children"}{"Why"} = [\%aydata];
			    } elsif($t eq "escalate") {
				# Old format is
				#  to
				
				my %edata = ("name" => "SurvivorAlertEscalation",
					     "text" => "",
					     "attrs" => {"Version" => "1.0"},
					     "children" => {},
					     "childorder" => ["EscalateTo"],
					     "parent" => \%xmltree);
				$xmltree{"children"}{"SurvivorAlertEscalation"} =
				    [\%edata];
				
				$to = <INFILE>;
				chomp($to);
				
				if($to =~ /xml/) {
				    # We've seen this one already
				    $skip++;
				}
				
				my %etdata = ("name" => "EscalateTo",
					      "text" => $to,
					      "attrs" => {},
					      "children" => {},
					      "parent" => \%edata);
				$adata{"children"}{"EscalateTo"} = [\%etdata];
			    }

			    # Generate the tree
			    $sx->Generate(\*OUTFILE, \%xmltree);

			    close(OUTFILE);

			    # Reset ownership and permissions on new file

			    chown $statinfo[4], $statinfo[5], $newfile;
			    chmod $statinfo[2], $newfile;

			    $ok++;
			} 

			close(INFILE);
		    }

		    # Unlink the old file and move the new one into place

		    if($ok) {
			if(!$skip) {
			    unlink($hfile);
			    rename($newfile, $hfile);
			} else {
			    # Don't complain if it's already converted
			    unlink($newfile);
			}
		    } else {
			# Don't complain if the file doesn't exist
			if(-e $hfile) {
			    printf("Error converting $hfile\n");
			}
		    }
		}
	    }
	}

	$done++;
    }
}

# Finally, convert the state/calllist tree

printf("Converting calllist state...\n");
$statedir = $ARGV[0] . "/calllist";

if(opendir(STATEDIR, $statedir)) {
    @calllists = grep(!/^\.\.?/, readdir(STATEDIR));
    closedir(STATEDIR);

    foreach $c (@calllists) {
	$cldir = $statedir . "/" . $c;

	printf("Processing $c\n");

	$clfile = $cldir . "/callliststatus";
	$lnfile = $cldir . "/lastnotify";
	$lrfile = $cldir . "/lastrotate";
	$sfile = $cldir . "/substitutions";

	$skipcls = 0;  # If previously converted
	
	my $sx = new Survivor::XML();

	# Everything starts the same
	my %xmltree = ("name" => "xml",
		       "text" => "",
		       "attrs" => {},
		       "children" => {},
		       "parent" => undef);

	my %cldata = ("name" => "SurvivorCallListStatus",
		      "text" => "",
		      "attrs" => {"Version" => "1.0"},
		      "children" => {},
		      "childorder" => ["LastNotify", "LastRotate"],
		      "parent" => \%xmltree);
	$xmltree{"children"}{"SurvivorCallListStatus"} = [\%cldata];
	
	# Consolidate lastnotify and lastrotate into callliststatus

	if(open(LNIN, $lnfile)) {
	    @statinfo = stat(LNIN);
	    
	    # lastnotify format:
	    #  person\n
	    #  address\n
	    #  module\n

	    $pr = <LNIN>;
	    chomp($pr);
	    $ad = <LNIN>;
	    chomp($ad);
	    $md = <LNIN>;
	    chomp($md);

	    my %lndata = ("name" => "LastNotify",
			  "text" => "",
			  "attrs" => {},
			  "children" => {},
			  "childorder" => ["Person", "Address", "Module"],
			  "parent" => \%cldata);
	    $cldata{"children"}{"LastNotify"} = [\%lndata];
	    
	    my %lnpdata = ("name" => "Person",
			   "text" => $pr,
			   "attrs" => {},
			   "children" => {},
			   "parent" => \%lndata);
	    $lndata{"children"}{"Person"} = [\%lnpdata];
	    
	    my %lnadata = ("name" => "Address",
			   "text" => $ad,
			   "attrs" => {},
			   "children" => {},
			   "parent" => \%lndata);
	    $lndata{"children"}{"Address"} = [\%lnadata];
	    
	    my %lnmdata = ("name" => "Module",
			   "text" => $md,
			   "attrs" => {},
			   "children" => {},
			   "parent" => \%lndata);
	    $lndata{"children"}{"Module"} = [\%lnmdata];
	
	    close(LNIN);
	} else {
	    # We've probably converted this already, or we could be
	    # looking at a broadcast list
	    
	    $skipcls++;
	}

	if(!$skipcls && open(LRIN, $lrfile)) {
	    # This file might not exist since only rotating calllists use it
	    # lastrotate format
	    #  time\n

	    $rt = <LRIN>;
	    chomp($rt);

	    my %lrdata = ("name" => "LastRotate",
			  "text" => $rt,
			  "attrs" => {},
			  "children" => {},
			  "parent" => \%cldata);
	    $cldata{"children"}{"LastRotate"} = [\%lrdata];
	    
	    close(LRIN);
	}

	if(!$skipcls && open(OUTFILE, "> $clfile")) {
	    # Generate the tree
	    $sx->Generate(\*OUTFILE, \%xmltree);
	    
	    close(OUTFILE);

	    # Reset ownership and permissions on new file
	    
	    chown $statinfo[4], $statinfo[5], $clfile;
	    chmod $statinfo[2], $clfile;

	    # Remove the old files

	    unlink($lnfile);
	    unlink($lrfile);
	}

	# Convert substitutions

	$skipsub = 0;

	if(open(SUBFILE, $sfile)) {
	    @statinfo = stat(SUBFILE);

	    $newsfile = $sfile . ".new";

	    if(open(OUTFILE, "> $newsfile")) {
		# Copy and convert the contents from the old file to the new

                # Everything starts the same
		my %sxmltree = ("name" => "xml",
				"text" => "",
				"attrs" => {},
				"children" => {},
				"parent" => undef);

		my %clsdata = ("name" => "SurvivorCallListSubstitutions",
			       "text" => "",
			       "attrs" => {"Version" => "1.0"},
			       "children" => {},
			       "childorder" => ["Substitution"],
			       "parent" => \%sxmltree);
		$sxmltree{"children"}{"SurvivorCallListSubstitutions"} =
		    [\%clsdata];
	
		while(<SUBFILE>) {
		    # Format is 0 or more lines of the form
		    #  begin:end:newname:oldname

		    my $x = $_;
		    chomp($x);
		    
		    my($sb,$se,$sr,$so) = split(/:/, $x);

		    if($sb =~ /xml/) {
			# We've probably done this already

			$skipsub++;
		    } else {
			my %sdata = ("name" => "Substitution",
				     "text" => "",
				     "attrs" => {},
				     "children" => {},
				     "childorder" => ["Begin", "End",
						      "Original",
						      "Replacement"],
				     "parent" => \%clsdata);
			push(@{$clsdata{"children"}{"Substitution"}},
			     \%sdata);

			my %sbdata = ("name" => "Begin",
				      "text" => $sb,
				      "attrs" => {},
				      "children" => {},
				      "parent" => \%sdata);
			$sdata{"children"}{"Begin"} = [\%sbdata];

			my %sedata = ("name" => "End",
				      "text" => $se,
				      "attrs" => {},
				      "children" => {},
				      "parent" => \%sdata);
			$sdata{"children"}{"End"} = [\%sedata];

			my %sodata = ("name" => "Original",
				      "text" => $so,
				      "attrs" => {},
				      "children" => {},
				      "parent" => \%sdata);
			$sdata{"children"}{"Original"} = [\%sodata];

			my %srdata = ("name" => "Replacement",
				      "text" => $sr,
				      "attrs" => {},
				      "children" => {},
				      "parent" => \%sdata);
			$sdata{"children"}{"Replacement"} = [\%srdata];
		    }
		}

		# Generate the tree
		$sx->Generate(\*OUTFILE, \%sxmltree);
		
		close(OUTFILE);

		# Reset ownership and permissions on new file
		
		chown $statinfo[4], $statinfo[5], $newsfile;
		chmod $statinfo[2], $newsfile;

		if(!$skipsub) {
		    unlink($sfile);
		    rename($newsfile, $sfile);
		} else {
		    # Don't complain if it's already converted
		    unlink($newsfile);
		}
	    }

	    close(SUBFILE);
	}
    }
}
