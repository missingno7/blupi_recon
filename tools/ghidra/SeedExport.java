import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class SeedExport extends GhidraScript {
    private String clean(String s) { return s.replace("*/", "* /").replace('\n', ' ').replace('\r', ' '); }
    public void run() throws Exception {
        String[] a = getScriptArgs();
        if (a.length != 2) throw new IllegalArgumentException("expected seed TSV and output directory");
        File outDir = new File(a[1]); outDir.mkdirs();
        Map<String,PrintWriter> writers = new HashMap<String,PrintWriter>();
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        int seeded = 0, exported = 0;
        try {
            for (String line : java.nio.file.Files.readAllLines(new File(a[0]).toPath(), StandardCharsets.UTF_8)) {
                if (line.trim().isEmpty()) continue;
                String[] x = line.split("\t", -1); if (x.length != 8) continue;
                long rva = Long.parseLong(x[0], 16), size = Long.parseLong(x[1]);
                Function f = getFunctionAt(currentProgram.getImageBase().add(rva));
                if (f == null) f = createFunction(currentProgram.getImageBase().add(rva), x[2]);
                if (f == null || f.isExternal()) continue;
                seeded++;
                if (!x[2].equals(f.getName())) try { f.setName(x[2], SourceType.USER_DEFINED); } catch (Exception ignored) {}
                if ("thiscall".equals(x[7])) try { f.setCallingConvention("__thiscall"); } catch (Exception ignored) {}
                String tu = new File(x[3]).getName();
                int dot = tu.lastIndexOf('.'); if (dot >= 0) tu = tu.substring(0, dot);
                tu += ".c";
                PrintWriter w = writers.get(tu);
                if (w == null) {
                    w = new PrintWriter(new OutputStreamWriter(new FileOutputStream(new File(outDir, tu)), StandardCharsets.UTF_8));
                    writers.put(tu, w);
                }
                w.printf("/* RVA=0x%X size=%d tu=%s best_source=%s map_status=%s accepted=%s cc=%s */%n",
                    rva, size, clean(x[3]), clean(x[4]), clean(x[5]), clean(x[6]), clean(x[7]));
                DecompileResults result = d.decompileFunction(f, 45, monitor);
                if (result.decompileCompleted()) w.println(result.getDecompiledFunction().getC());
                else w.printf("/* DECOMPILE FAILED: %s */%n", clean(result.getErrorMessage()));
                exported++;
            }
        } finally {
            d.dispose(); for (PrintWriter w : writers.values()) { w.close(); }
        }
        println("SeedExport: seeded=" + seeded + " exported=" + exported + " TU files=" + writers.size());
    }
}
