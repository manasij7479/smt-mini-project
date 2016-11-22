pre: a==1
{
  def: foo {
    a = a + 1;
    assert: !(a==0)
  }
  call: foo
  assert: a==2
  a = a+1;
}
post: a==3
