#!/usr/bin/perl -w

my $last;
my $hours;
my $Id;
my $race;
my @matches;
my @match2;
my @match3;
my @match4;
my @match5;
my @match6;
my @match7;
my @vamps;
my @garou;
my @fae;
my @mortal;
my @undef;
my %IdHash;

chdir('../player');
system('gzip -dfq *');

@pfiles = grep { -f } <*>;

foreach $file (@pfiles) {
    open PFILE, $file;
    while (<PFILE>) {
	if(m/^LogO (\d+)\n$/m) {
	    $last = $1;
	} elsif (m/^Plyd (\d+)\n$/m) {
	    $hours = $1;
	} elsif (m/^Id   (\d+)\n$/m) {
	    $Id = $1;
	} elsif (m/^Race (\d)\n$/m) {
	    $race = $1;
	}
    }
    if($last && $hours)
    {
	if($last < (time - 60*60*24*365) && $hours < 10*60*60) {
	    push @matches, $file;
	}
	if($last < (time - 60*60*24*60) && $hours < 60*60) {
	    push @matches, $file;
	}
	if($last > (time - 60*60*24*365) && $hours < 10*60*60) {
	    push @match2, $file;
	}
	if($last < (time - 60*60*24*365) && $hours >= 10*60*60) {
	    push @match3, $file;
	}
	if($last > (time - 60*60*24*30) && $hours >= 10*60*60) {
	    push @match6, $file;
	}
    }
    if($last)
    {
	if($last > (time - 60*60*24*7)) {
	    push @match4, $file;
	}
	if($last > (time - 60*60*24*30)) {
	    push @match5, $file;
	}
	    if($race == 1 && $last > (time - 60*60*24*30)) {
		push @mortal, $file;
	    } elsif($race == 2 && $last > (time - 60*60*24*30)) {
		push @garou, $file;
	    } elsif($race == 3 && $last > (time - 60*60*24*30)) {
		push @vamps, $file;
	    } elsif($race == 4 && $last > (time - 60*60*24*30)) {
		push @fae, $file;
	    } elsif($last > (time - 60*60*24*30)) {
		push @undef, $file;
	    }
    }
    if($Id)
    {
	if(defined($IdHash{$Id}))
	{
	    push @match7, $file;
	    push @match7, $IdHash{$Id};
	}
	else
	{
	    $IdHash{$Id} = $file;
	}
    }
    $last = 0;
    $hours = 0;
    $Id = 0;
    close PFILE;
}

$temp = @matches;
$temp2 = @pfiles - $temp;

foreach $file (@matches) {
    unlink $file;
}

print $temp." player files deleted.\n\n";

$temp = @match2;
print $temp."/".$temp2." player files with less than 10 hours.\n";

$count = 0;
$temp = @match3;
print $temp."/".$temp2." players which have not logged on in over a year.\n";
print "Those players are:\n";
foreach $file (@match3) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
    system("gzip -fq $file");
}
print "\n\n";

$count = 0;
$temp = @match4;
print $temp."/".$temp2." players which have logged on in the past week.\n";
print "Those players are:\n";
foreach $file (@match4) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @match5;
print $temp."/".$temp2." players which have logged on in the past 30 days.\n";
$count = 0;
$temp = @match6;
print $temp." of whom have been on more than 10 hours.\n";
print "Those players are:\n";
foreach $file (@match6) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @vamps;
$count = 0;
print $temp." of whom are vampires.\n";
print "Those players are:\n";
foreach $file (@vamps) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @garou;
$count = 0;
print $temp." of whom are garou.\n";
print "Those players are:\n";
foreach $file (@garou) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @fae;
$count = 0;
print $temp." of whom are changelings.\n";
print "Those players are:\n";
foreach $file (@fae) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @mortal;
$count = 0;
print $temp." of whom are mortals.\n";
print "Those players are:\n";
foreach $file (@mortal) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @undef;
$count = 0;
print $temp." of whom are not a player race.\n";
print "Those players are:\n";
foreach $file (@undef) {
    print $file." ";
    if(++$count == 8) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";

$temp = @match7;
print $temp."/".$temp2." pfiles which may have suffered from the name bug.\n";
$count = 0;
print "Those players are:\n";
foreach $file (@match7) {
    print $file." ";
    if(++$count%2 == 0) { print "\t"; }
    if($count == 4) {
	print "\n";
	$count = 0;
    }
}
print "\n\n";
