passed = False
try:
    print(a)
except NameError:
    passed = True
if not passed:
    raise RuntimeError("'a' is defined after reset.")
