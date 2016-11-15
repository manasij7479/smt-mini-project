pre: z == 6
{
  if (x >= 0) {
    y = z;
  } else {
    if ( x == -1) {
      y = 100;
    } else {
      if (x == -2) {
        y = 5+1;
      } else {
        y = -z;
      }
    }
  }
}
post: y>5
