tables = ["MAGIC","MAGIC_TYPE1","MAGIC_TYPE2","MAGIC_TYPE3","MAGIC_TYPE4",
          "MAGIC_TYPE5","MAGIC_TYPE6","MAGIC_TYPE7","MAGIC_TYPE8","MAGIC_TYPE9"]
out = ["SET NOCOUNT ON;"]
for t in tables:
    out.append("PRINT '==%s==';" % t)
    out.append("SELECT * FROM %s;" % t)
out.append("GO")
open("/tmp/dumpmagic.sql","w").write("\n".join(out)+"\n")
print("\n".join(out[:4]))
