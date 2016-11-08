pre: x == 1
{
  if ((x > 10) && (x > 2)) {
    x = x + 1;
  } else {
    if ((x > 4) || (x == 5)) {
      x = x - 1;
    } else {
      x = x - 2;
    }  
  }
}
post: x == 2
