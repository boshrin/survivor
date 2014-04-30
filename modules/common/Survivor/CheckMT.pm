# Survivor::CheckMT.pm
#
# Version: $Revision: 0.8 $
# Author: Benjamin Oshrin
# Date: $Date: 2006/09/11 02:41:32 $
#
# MultiThreaded routines for Survivor.pm.  These are in a separate module
# because it is much, much, much easier that way.
#
# Copyright (c) 2002 - 2006
# The Trustees of Columbia University in the City of New York
# Academic Information Systems
# 
# License restrictions apply, see doc/license.html for details.

package Survivor::CheckMT;

use strict;
use Survivor;

use vars qw( $doduration );

=head1 NAME

Survivor::CheckMT contains the multi-threaded routines for Survivor.pm.

=head1 SYNOPSIS

 <This module should not be accessed directly>

=head1 CONSTRUCTOR

=over 4

=item new ()

Creates a C<Survivor::CheckMT>, which is a reference to a newly created
Survivor::CheckMT object.  C<new()> takes no arguments.

=back

=cut

sub new {
    my $self = {};
    bless $self;

    # Initialize flags
    $doduration = 1;;            # Measure execution duration
    
    return $self;
}

=head1 METHODS

=over 4

=item ExecMT (SC, CHECK)

Multithreaded implementation of Check::Exec.

  Arguments
  =========
  SC: A Survivor::Check object.
  CHECK: A Check subroutine.

  Returns
  =======
  The largest return value generated.

=back

=cut

sub ExecMT {
    require threads;
    require threads::shared;
    require Thread::Queue;
    require Thread::Semaphore;
    
    import threads;
    import threads::shared;
    import Thread::Queue;
    import Thread::Semaphore;

    my $self = shift;  # Survivor::CheckMT object
    my $sc = shift;    # Survivor::Check object
    my $csub = shift;  # Check subroutine

    my $ret = MODEXEC_OK;
    
    # Thread related stuff.  In order to access these, the subroutines
    # we call must be in this scope.  The DEFAULT_CMTHREADS is
    # defined here to be the same as in libcm.H.
    
    my $DEFAULT_CMTHREADS = 15;
    my %tids;

    my $todo = new Thread::Queue;
    my $outsem = new Thread::Semaphore;  # Output semaphore
    
    # Push each host to be checked onto the queue
    
    foreach my $h ($sc->Hosts()) {
	$todo->enqueue($h);
    }
    
    # Launch the threads, up to a maximum of $DEFAULT_CMTHREADS;
    
    for(my $i = 0;
	$i < (scalar($sc->Hosts()) < $DEFAULT_CMTHREADS ?
	      scalar($sc->Hosts()) : $DEFAULT_CMTHREADS);
	$i++) {
	$tids{$i} = threads->new(\&RunThread, $csub, \$todo, $sc, $i,
				 \$outsem);
    }
        
    # Join the threads and clean up
    
    for(my $i = 0;
	$i < (scalar($sc->Hosts()) < $DEFAULT_CMTHREADS ?
	      scalar($sc->Hosts()) : $DEFAULT_CMTHREADS);
	$i++) {
	# Use the return values of each thread to determine the
	# largest return value
	my $r = $tids{$i}->join;

	if($r > $ret) {
	    $ret = $r;
	}
    }

    return($ret);
    
    sub RunThread {
	my ($crunsub, $todo, $sc, $index, $outsem) = (@_);
	my $maxrv = MODEXEC_OK;
	
	while(my $host = $$todo->dequeue_nb) {
	    if(defined $dlog) {
		$dlog->log_progress("$0 execing via thread", $host);
	    }
	    
	    my $start = -1;
	    my $end = -1;

	    if($doduration) { $start = time(); }
	    
	    my ($rv, $scl, $cmt, $dur) = &$crunsub($sc, $host);

	    if($rv > $maxrv) {
		$maxrv = $rv;
	    }
	    
	    if($doduration) {
		$end = time();

		$dur = ($end - $start) * 1000;
	    }

	    # We have to be careful not to interleave output with
	    # other threads, so down the semaphore before dumping
	    # the result

	    $$outsem->down;
	    $sc->Result($host, $rv, $scl, $cmt, $dur);
	    $$outsem->up;
	}

	return($maxrv);
    }
}
    
=head1 BUGS

No known bugs.

=head1 AUTHORS

Benjamin Oshrin E<lt>F<benno@columbia.edu>E<gt>

=head1 COPYRIGHT

Copyright (c) 2002 - 2006
The Trustees of Columbia University in the City of New York
Academic Information Systems

License restrictions apply, see F<doc/license.html> for details.

=cut

1;
