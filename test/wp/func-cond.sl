pre: z == 6
{
  def: foo {
    if (x >= 0) {
      y = z;
    } else {
      y = -z;
    }
  }
  call: foo
}
post: y>5
