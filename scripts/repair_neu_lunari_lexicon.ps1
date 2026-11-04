param(
  [string]$LexiconPath = (Join-Path $PSScriptRoot '..\neu-lunari-german-v1.tsv')
)

$ErrorActionPreference = 'Stop'
$LexiconPath = [IO.Path]::GetFullPath($LexiconPath)
if (-not (Test-Path -LiteralPath $LexiconPath)) { throw "V2-Lexikon nicht gefunden: $LexiconPath" }

if (-not ('NeuLunariLexiconRepair' -as [type])) {
  Add-Type @'
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

public static class NeuLunariLexiconRepair {
  static string Normalize(string value) {
    return value.Trim().ToLowerInvariant().Replace("ä", "ae").Replace("ö", "oe").Replace("ü", "ue").Replace("ß", "ss");
  }
  static ulong Hash(string word, ulong salt) {
    unchecked {
      ulong hash = 1469598103934665603UL ^ salt;
      foreach (byte b in Encoding.UTF8.GetBytes(word)) { hash ^= b; hash *= 1099511628211UL; }
      return hash;
    }
  }
  static string Form(ulong hash) {
    string[] onsets = {"m","n","l","r","p","t","k","b","d","g","f","s","v","h"};
    string[] vowels = {"a","e","i","o","u"};
    var value = new StringBuilder();
    for (int i = 0; i < 4; ++i) { value.Append(onsets[hash % (ulong)onsets.Length]); hash /= (ulong)onsets.Length; value.Append(vowels[hash % (ulong)vowels.Length]); hash /= (ulong)vowels.Length; }
    return value.ToString();
  }
  static bool Reserved(string form) {
    var forms = new HashSet<string> {"mer","gav","sel","se","gal","rin","lan","sir","san","ke","va","vas","ya","na","ren","sil","lir","sen","tan","nus","nor","vel","tel","sar","lin","kan","ves","dar","mur","res","vin","lum","nel","tal","sor","rem","ket","val","tem","sur","bel","pel","mir","dam","man","min","lon","tir","kar","sev","num","var","sam","som","nam","pur","sul","rav","hes","fik","fel","saf","gol","ran","lor","bas","kos","kim","lus","suv","tim","ki","ku","ril","rul","qi","qu","fim","loma"};
    return forms.Contains(form);
  }
  public static int Repair(string inputPath) {
    var tempPath = inputPath + ".repairing";
    var backupPath = inputPath + ".before-collision-repair.bak";
    var utf8 = new UTF8Encoding(false);
    var forms = new Dictionary<string,string>(StringComparer.Ordinal);
    var words = new Dictionary<string,string>(StringComparer.Ordinal);
    int repaired = 0;
    using (var reader = new StreamReader(inputPath, Encoding.UTF8, true))
    using (var writer = new StreamWriter(tempPath, false, utf8)) {
      string header = reader.ReadLine(); string columns = reader.ReadLine();
      if (header == null || !header.Contains("Neu-Lunari v2") || columns == null) throw new InvalidDataException("Ungültiges V2-Lexikon.");
      writer.WriteLine(header); writer.WriteLine(columns);
      string line;
      while ((line = reader.ReadLine()) != null) {
        int tab = line.IndexOf('\t'); if (tab < 1) continue;
        string original = line.Substring(0, tab).Trim(), word = Normalize(original), form = line.Substring(tab + 1).Trim();
        if (word.Length == 0 || form.Length == 0) continue;
        // German casing is not meaning-distinguishing in V2.  Keep the first
        // spelling only; otherwise one Lunari form would have several Duden
        // rows but could not have one unique reverse entry.
        if (words.TryGetValue(word, out string known)) continue;
        if (forms.ContainsKey(form) || Reserved(form)) {
          ulong salt = 0; string candidate;
          do { candidate = Form(Hash(word, salt++)); } while (forms.ContainsKey(candidate) || Reserved(candidate));
          form = candidate; ++repaired;
        }
        words.Add(word, form); forms.Add(form, word); writer.WriteLine(original + "\t" + form);
      }
    }
    File.Copy(inputPath, backupPath, true);
    File.Copy(tempPath, inputPath, true); File.Delete(tempPath);
    return repaired;
  }
}
'@
}

$repaired = [NeuLunariLexiconRepair]::Repair($LexiconPath)
Write-Output "Bereinigtes V2-Lexikon: $repaired Formkollisionen neu vergeben. Sicherung: $LexiconPath.before-collision-repair.bak"
