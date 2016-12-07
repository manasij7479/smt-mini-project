pre: 'a == '100
{
  if ('a '> '100) {
    'a = 'a;
  }
  else {
    'a = 'a '+ '1;
  }
}
post: 'a '> '9
