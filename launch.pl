$run = $ARGV[0];
$par1 = $ARGV[1];
print("run: ".$run."\n");

if( ! -e "data/rec_capture_".$run.".txt" )
{
  system("unzip -u -d data/ data/rec_capture_".$run.".zip");
}

if( ! -e "ntuples/tot_capture_".$run.".root" )
{
  system("./PADEToRoot.exe ".$run." ".$par1);
}

$dir = "/afs/cern.ch/user/a/abenagli/www/TBatFNAL/".$run;
system("mkdir ".$dir);
system("cp ~/public/index.php ".$dir);

$subdir1 = $dir."/plotsPerFib";
system("mkdir ".$subdir1);
system("cp ~/public/index.php ".$subdir1);

system("./DQMPlots.exe ".$run);
