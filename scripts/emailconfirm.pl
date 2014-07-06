#!/usr/bin/perl -w
# Usage: emailconfirm.pl TO CHAR_NAME CONFIRM_CODE

use Socket;
#use strict;

my(@digital);
#my($length);
my($packFormat);

my($mailTo)     = $ARGV[0];

my($mailServer) = 'localhost';

my($mailFrom)   = 'projecttwilight@brandonsplace.net;
my($realName)   = "Project Twilight Mud";
my($subject)    = "Confirm your email for $ARGV[1]";
my($body)       = 'Please confirm your email address for ' . $ARGV[1] . '.\nLog in to Project Twilight at brandonsplace.net port 9080, log in with your character, ' . $ARGV[1] . ', and enter the confirmation code ' . $ARGV[2] . '.\n\nIf this is not your character, please email ptrayal@gmail.com';


$main::SIG{'INT'} = 'closeSocket';


my($proto)      = getprotobyname("tcp")        || 6;
my($port)       = getservbyname("SMTP", "tcp") || 25;
my($name, $aliases, $type, $length, $serverAddr) = (gethostbyname($mailServer));

if (! defined($serverAddr)) {

    die('gethostbyname failed.');
}


socket(SMTP, AF_INET(), SOCK_STREAM(), $proto)
    or die("socket: $!");

$packFormat = 'S n a4 x8';   # Windows 95, SunOs 4.1+
#$packFormat = 'S n c4 x8';   # SunOs 5.4+ (Solaris 2)

$serverAddr = pack('C4',127,0,0,1);
connect(SMTP, pack($packFormat, AF_INET(), $port, $serverAddr))
    or die("connect: $!");


select(SMTP); $| = 1; select(STDOUT);    # use unbuffemiles i/o.


{
    my($inpBuf) = '';


    recv(SMTP, $inpBuf, 200, 0);
    recv(SMTP, $inpBuf, 200, 0);
}


sendSMTP(1, "HELO\n");
sendSMTP(1, "MAIL From: <$mailFrom>\n");
sendSMTP(1, "RCPT To: <$mailTo>\n");
sendSMTP(1, "DATA\n");


send(SMTP, "From: $realName\n", 0);
send(SMTP, "Subject: $subject\n", 0);
send(SMTP, $body, 0);


sendSMTP(1, "\r\n.\r\n");
sendSMTP(1, "QUIT\n");


close(SMTP);


sub closeSocket {     # close smtp socket on error
    close(SMTP);
    die("SMTP socket closed due to SIGINT\n");
}


sub sendSMTP {
    my($debug)  = shift;
    my($buffer) = @_;


    print STDERR ("> $buffer") if $debug;
    send(SMTP, $buffer, 0);


    recv(SMTP, $buffer, 200, 0);
    print STDERR ("< $buffer") if $debug;


    return( (split(/ /, $buffer))[0] );
}
