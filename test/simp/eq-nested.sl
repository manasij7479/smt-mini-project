pre: x == 1
{
  y = 0;
  if (x > 1) {
    x = x + 1;
  } else {
    if (y > 100) {
      x = x + 1;
    } else {
      x = x + 1;
    }
    if ( (x > 3) && (x < 3) ) {
      x = x + 1;
    }
  }
}
post: x == 2
