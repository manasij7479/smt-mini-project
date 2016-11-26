pre: (a==1) && (b==2)
{
  def: foo {
    c = a;
    a = b;
    b = c;
  }
  call: foo
  call: foo
}
post: (a==1) && (b==2)
