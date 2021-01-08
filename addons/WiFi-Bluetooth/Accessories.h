// -----------------------------------------------------------------------------------
// Misc functions to help with commands, etc.
#pragma once

// integer numeric conversion with error checking
boolean atoi2(char *a, int *i) {
  char *conv_end;
  long l=strtol(a,&conv_end,10);
  
  if ((l<-32767) || (l>32768) || (&a[0]==conv_end)) return false;
  *i=l;
  return true;
}

char serialRecvFlush() {
  char c=0;
  while (Ser.available()>0) c=Ser.read();
  return c;
}

// smart LX200 aware command and response over serial
boolean processCommand(const char cmd[], char response[], long timeOutMs) {
  Ser.setTimeout(timeOutMs);
  
  // clear the read/write buffers
  Ser.flush();
  serialRecvFlush();

  // send the command
  Ser.print(cmd);

  boolean noResponse=false;
  boolean shortResponse=false;
  if ((cmd[0]==(char)6) && (cmd[1]==0)) shortResponse=true;
  if ((cmd[0]==':') || (cmd[0]==';')) {
    if (cmd[1]=='G') {
      if (strchr("RDE",cmd[2])) { if (timeOutMs < 300) timeOutMs = 300; }
    } else
    if (cmd[1]=='M') {
      if (strchr("ewnsg",cmd[2])) noResponse=true;
      if (strchr("SAP",cmd[2])) shortResponse=true;
    } else
    if (cmd[1]=='Q') {
      if (strchr("#ewns",cmd[2])) noResponse=true;
    } else
    if (cmd[1]=='A') {
      if (strchr("W123456789+",cmd[2])) { shortResponse=true; if (timeOutMs < 1000) timeOutMs = 1000; }
    } else
    if ((cmd[1]=='F') || (cmd[1]=='f')) {
      if (strchr("+-QZHhF1234",cmd[2])) noResponse=true;
      if (strchr("Ap",cmd[2])) shortResponse=true;
    } else
    if (cmd[1]=='r') {
      if (strchr("+-PRFC<>Q1234",cmd[2])) noResponse=true;
      if (strchr("~S",cmd[2])) shortResponse=true;
    } else
    if (cmd[1] == 'R') {
      if (strchr("AEGCMS0123456789", cmd[2])) noResponse=true;
    } else
    if (cmd[1]=='S') {
      if (strchr("CLSGtgMNOPrdhoTBX",cmd[2])) shortResponse=true;
    } else
    if (cmd[1]=='L') {
      if (strchr("BNCDL!",cmd[2])) noResponse=true;
      if (strchr("o$W",cmd[2])) { shortResponse=true; if (timeOutMs < 1000) timeOutMs = 1000; }
    } else
    if (cmd[1]=='B') {
      if (strchr("+-",cmd[2])) noResponse=true;
    } else
    if (cmd[1]=='C') {
      if (strchr("S",cmd[2])) noResponse=true;
    } else
    if (cmd[1]=='h') {
      if (strchr("FC",cmd[2])) { noResponse=true; if (timeOutMs < 1000) timeOutMs = 1000; }
      if (strchr("QPR",cmd[2])) { shortResponse=true; if (timeOutMs < 300) timeOutMs = 300; }
    } else
    if (cmd[1]=='T') {
      if (strchr("QR+-SLK",cmd[2])) noResponse=true;
      if (strchr("edrn",cmd[2])) shortResponse=true;
    } else
    if (cmd[1]=='U') {
      noResponse=true; 
    } else
    if ((cmd[1]=='W') && (cmd[2]!='?')) { 
      noResponse=true; 
    } else
    if ((cmd[1]=='$') && (cmd[2]=='Q') && (cmd[3]=='Z')) {
      if (strchr("+-Z/!",cmd[4])) noResponse=true;
    }

    // override for checksum protocol
    if (cmd[0]==';') { noResponse=false; shortResponse=false; }
  }

  unsigned long timeout=millis()+(unsigned long)timeOutMs;
  if (noResponse) {
    response[0]=0;
    return true;
  } else
  if (shortResponse) {
    while ((long)(timeout-millis()) > 0) {
      if (Ser.available()) { response[Ser.readBytes(response,1)]=0; break; }
    }
    return (response[0]!=0);
  } else {
    // get full response, '#' terminated
    int responsePos=0;
    char b=0;
    while ((long)(timeout-millis()) > 0 && b != '#') {
      if (Ser.available()) {
        b=Ser.read();
        response[responsePos]=b; responsePos++; if (responsePos>39) responsePos=39; response[responsePos]=0;
      }
    }
    return (response[0]!=0);
  }
}

bool command(const char command[], char response[]) {
  bool success = processCommand(command,response,webTimeout);
  int l=strlen(response)-1; if (l >= 0 && response[l] == '#') response[l]=0;
  return success;
}

bool commandBlind(const char command[]) {
  char response[40]="";
  return processCommand(command,response,webTimeout);
}

bool commandEcho(const char command[]) {
  char response[40]="";
  char c[40]="";
  sprintf(c,":EC%s#",command);
  return processCommand(c,response,webTimeout);
}

bool commandBool(const char command[]) {
  char response[40]="";
  bool success = processCommand(command,response,webTimeout);
  int l=strlen(response)-1; if (l >= 0 && response[l] == '#') response[l]=0;
  if (!success) return false;
  if (response[1] != 0) return false;
  if (response[0] == '0') return false; else return true;
}

char *commandString(const char command[]) {
  static char response[40]="";
  bool success = processCommand(command,response,webTimeout);
  int l=strlen(response)-1; if (l >= 0 && response[l] == '#') response[l]=0;
  if (!success) strcpy(response,"?");
  return response;
}

boolean doubleToDms(char *reply, double *f, boolean fullRange, boolean signPresent) {
  char sign[]="+";
  int  o=0,d1,s1=0;
  double m1,f1;
  f1=*f;

  // setup formatting, handle adding the sign
  if (f1<0) { f1=-f1; sign[0]='-'; }

  f1=f1+0.000139; // round to 1/2 arc-second
  d1=floor(f1);
  m1=(f1-d1)*60.0;
  s1=(m1-floor(m1))*60.0;
  
  char s[]="+%02d*%02d:%02d";
  if (signPresent) { 
    if (sign[0]=='-') { s[0]='-'; } o=1;
  } else {
    strcpy(s,"%02d*%02d:%02d");
  }
  if (fullRange) s[2+o]='3';
 
  sprintf(reply,s,d1,(int)m1,s1);
  return true;
}
