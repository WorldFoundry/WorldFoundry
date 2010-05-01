class a {
  virtual int q();
};
class b : public a {
  int r;
};

main() {
  a *mya = new b;

  if(dynamic_cast<b*>(mya)) { printf("see?\n");}
}
