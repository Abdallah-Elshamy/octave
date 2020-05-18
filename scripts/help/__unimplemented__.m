########################################################################
##
## Copyright (C) 2010-2020 The Octave Project Developers
##
## See the file COPYRIGHT.md in the top-level directory of this
## distribution or <https://octave.org/copyright/>.
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.
##
########################################################################

## -*- texinfo -*-
## @deftypefn {} {@var{txt} =} __unimplemented__ (@var{fcn})
## Return specific help text for the unimplemented function @var{fcn}.
##
## This is usually a suggestion for an existing compatible function to use in
## place of @var{fcn}.
##
## This function is not called by users, but by the Octave interpreter when it
## fails to recognize an input string as a valid function name.  See
## @code{missing_function_hook} for using a different handler for this event.
## @seealso{missing_function_hook}
## @end deftypefn


function txt = __unimplemented__ (fcn)

  if (nargin != 1)
    print_usage ();
  endif

  is_matlab_function = true;

  ## Some smarter cases, add more as needed.
  switch (fcn)
    case {"avifile", "aviinfo", "aviread"}
      txt = ["Basic video file support is provided in the video package.  ", ...
             "See @url{https://octave.sourceforge.io/video/}."];

    case "funm"
      txt = ["funm is not currently part of core Octave.  ", ...
             "See the linear-algebra package at ", ...
             "@url{https://octave.sourceforge.io/linear-algebra/}."];

    case "griddedInterpolant"
      txt = ["griddedInterpolant is not implemented.  ", ...
             "Consider using griddata."];

    case "linprog"
      txt = ["Octave does not currently provide linprog.  ", ...
             "Linear programming problems may be solved using @code{glpk}.  ", ...
             "Try @code{help glpk} for more info."];

    case "matlabrc"
      txt = ["matlabrc is not implemented.  ", ...
             "Octave uses the file '.octaverc' instead."];

    case {"ode113", "ode23t", "ode23tb"}
      txt = ["Octave provides lsode and ode15i, ode15s, ode23, ode23s, ", ...
             "and ode45 for solving differential equations.  For more", ...
             "information try @code{help lsode}, @code{help ode45}.  ", ...
             "Matlab-compatible ODE functions are provided by the odepkg ", ...
             "package.  See @url{https://octave.sourceforge.io/odepkg/}."];

    case "polarplot"
      txt = ["polarplot is not implemented.  Consider using polar."];

    case "startup"
      txt = ["'startup.m' is a user startup and configuration script.  ", ...
             "Try @code{doc startup} for more information."];

    case {"xlsread", "xlsfinfo", "xlswrite", "wk1read", "wk1finfo", "wk1write"}
      txt = ["Functions for spreadsheet style I/O ", ...
             "(.xls .xlsx .sxc .ods .dbf .wk1 etc.) " , ...
             "are provided in the io package. ", ...
             "See @url{https://octave.sourceforge.io/io/}."];

    ## control system
    case {"absorbDelay", "allmargin", "append", "augstate", "balreal", ...
          "balred", "balredOptions", "bdschur", "bode", ...
          "bodemag", "bodeoptions", "bodeplot", "c2d", "c2dOptions", ...
          "canon", "care", "chgFreqUnit", "chgTimeUnit", "connect", ...
          "connectOptions", "covar", "ctrb", "ctrbf", "ctrlpref", "d2c", ...
          "d2cOptions", "d2d", "d2dOptions", "damp", "dare", "db2mag", ...
          "dcgain", "delay2z", "delayss", "dlqr", "dlyap", "dlyapchol", ...
          "drss", "dsort", "dss", "dssdata", "esort", "estim", "evalfr", ...
          "feedback", "filt", "frd", "frdata", "freqresp", "gcare", "gdare", ...
          "genfrd", "genmat", "gensig", "genss", "get", "getBlockValue", ...
          "getDelayModel", "getGainCrossover", "getIOTransfer", ...
          "getLFTModel", "getLoopTransfer", "getNominal", "getoptions", ...
          "getPeakGain", "getSwitches", "getValue", "gram", "hasdelay", ...
          "hasInternalDelay", "hsvd", "hsvdOptions", "hsvoptions", ...
          "hsvplot", "imp2exp", "impulse", "impulseplot", "initial", ...
          "initialplot", "iopzmap", "iopzplot", "isct", "isdt", ...
          "isfinite", "isParametric", "isproper", "isreal", "issiso", ...
          "isstable", "isstatic", "kalman", "kalmd", "lft", "loopswitch", ...
          "lqg", "lqgreg", "lqgtrack", "lqi", "lqr", "lqrd", "lqry", "lsim", ...
          "lsiminfo", "lsimplot", "ltiview", "lyap", "lyapchol", "mag2db", ...
          "margin", "minreal", "modred", "modsep", "nblocks", "ndims", ...
          "ngrid", "nichols", "nicholsoptions", "nicholsplot", "nmodels", ...
          "norm", "nyquist", "nyquistoptions", "nyquistplot", "obsv", ...
          "obsvf", "order", "pade", "parallel", "permute", "pid", "piddata", ...
          "pidstd", "pidstddata", "pidtool", "pidtune", "pidtuneOptions", ...
          "place", "pole", "prescale", "pzmap", "pzoptions", "pzplot", ...
          "realp", "reg", "replaceBlock", "repsys", "reshape", "rlocus", ...
          "rlocusplot", "rss", "series", "set", "setBlockValue", ...
          "setDelayModel", "setoptions", "setValue", "sgrid", ...
          "showBlockValue", "showTunable", "sigma", "sigmaoptions", ...
          "sigmaplot", "sisoinit", "sisotool", "sminreal", "ss", ...
          "ss2ss", "ssdata", "stabsep", "stabsepOptions", "stack", "step", ...
          "stepDataOptions", "stepinfo", "stepplot", "sumblk", "tf", ...
          "tfdata", "thiran", "timeoptions", "totaldelay", "tzero", ...
          "updateSystem", "upsample", "xperm", "zero", "zgrid", "zpk", ...
          "zpkdata"}
      txt = check_package (fcn, "control");

    ## communications
    case {"algdeintrlv", "algintrlv", "alignsignals", "amdemod", "ammod", ...
          "arithdeco", "arithenco", "awgn", "bchdec", "bchenc", ...
          "bchgenpoly", "bchnumerr", "berawgn", "bercoding", "berconfint", ...
          "berfading", "berfit", "bersync", "bertool", "bi2de", "bin2gray", ...
          "biterr", "bsc", "cma", "commscope", "compand", "convdeintrlv", ...
          "convenc", "convintrlv", "convmtx", "cosets", "cyclgen", ...
          "cyclpoly", "de2bi", "decode", "deintrlv", "dfe", "dftmtx", ...
          "distspec", "doppler", "dpcmdeco", "dpcmenco", "dpcmopt", ...
          "dpskdemod", "dpskmod", "dvbs2ldpc", "encode", "equalize", ...
          "eyediagram", "EyeScope", "finddelay", "fmdemod", "fmmod", ...
          "fskdemod", "fskmod", "gaussdesign", "gen2par", "genqamdemod", ...
          "genqammod", "gf", "gfadd", "gfconv", "gfcosets", "gfdeconv", ...
          "gfdiv", "gffilter", "gflineq", "gfminpol", "gfmul", "gfpretty", ...
          "gfprimck", "gfprimdf", "gfprimfd", "gfrank", "gfrepcov", ...
          "gfroots", "gfsub", "gftable", "gftrunc", "gftuple", "gfweight", ...
          "gray2bin", "hammgen", "heldeintrlv", "helintrlv", ...
          "helscandeintrlv", "helscanintrlv", "huffmandeco", "huffmandict", ...
          "huffmanenco", "intdump", "intrlv", "iscatastrophic", ...
          "isprimitive", "istrellis", "legacychannelsim", "lineareq", ...
          "lloyds", "lms", "lteZadoffChuSeq", "marcumq", ...
          "mask2shift", "matdeintrlv", "matintrlv", "minpol", "mldivide", ...
          "mlseeq", "modnorm", "muxdeintrlv", "muxintrlv", "noisebw", ...
          "normlms", "oct2dec", "oqpskdemod", "oqpskmod", "pamdemod", ...
          "pammod", "pmdemod", "pmmod", "poly2trellis", "primpoly", ...
          "pskdemod", "pskmod", "qamdemod", "qammod", "qfunc", "qfuncinv", ...
          "quantiz", "randdeintrlv", "randerr", "randintrlv", "randsrc", ...
          "rayleighchan", "rcosdesign", "rectpulse", "reset", "ricianchan", ...
          "rls", "rsdec", "rsenc", "rsgenpoly", "rsgenpolycoeffs", ...
          "scatterplot", "sdrexamples", "sdrfgensysace", "sdrfroot", ...
          "sdrinfo", "sdrload", "sdrsetup", "semianalytic", "shift2mask", ...
          "signlms", "ssbdemod", "ssbmod", "stdchan", ...
          "supportPackageInstaller", "symerr", "syndtable", "varlms", ...
          "vec2mat", "vitdec", "wgn"}
      txt = check_package (fcn, "communications");

    ## finance
    case {"abs2active", "accrfrac", "acrubond", "acrudisc", "active2abs", ...
          "addEquality", "addGroupRatio", "addGroups", "addInequality", ...
          "adline", "adosc", "amortize", "annurate", "annuterm", ...
          "arith2geom", "ascii2fts", "beytbill", "binprice", "blkimpv", ...
          "blkprice", "blsdelta", "blsgamma", "blsimpv", "blslambda", ...
          "blsprice", "blsrho", "blstheta", "blsvega", "bndconvp", ...
          "bndconvy", "bnddurp", "bnddury", "bndkrdur", "bndprice", ...
          "bndspread", "bndtotalreturn", "bndyield", "bolling", "bollinger", ...
          "boxcox", "busdate", "busdays", "candle", "cdai", "cdprice", ...
          "cdyield", "cfamounts", "cfbyzero", "cfconv", "cfdates", "cfdur", ...
          "cfplot", "cfport", "cfprice", "cfspread", "cftimes", "cfyield", ...
          "chaikosc", "chaikvolat", "chartfts", "checkFeasibility", ...
          "chfield", "convert2sur", ...
          "convertto", "corr2cov", "cov2corr", "cpncount", "cpndaten", ...
          "cpndatenq", "cpndatep", "cpndaysp", "cpnpersz", "createholidays", ...
          "cumsum", "cur2frac", "cur2str", "date2time", "dateaxis", ...
          "datedisp", "datefind", "datemnth", "datewrkdy", "day", "days360", ...
          "days360e", "days360isda", "days360psa", "days365", "daysact", ...
          "daysadd", "daysdif", "dec2thirtytwo", "depfixdb", "depgendb", ...
          "deprdv", "depsoyd", "depstln", "diff", "disc2zero", "discrate", ...
          "ecmmvnrfish", "ecmmvnrmle", "ecmmvnrobj", "ecmmvnrstd", ...
          "ecmnfish", "ecmnhess", "ecmninit", "ecmnmle", "ecmnobj", ...
          "ecmnstd", "effrr", "elpm", "emaxdrawdown", "end", "eomdate", ...
          "estimateAssetMoments", "estimateBounds", ...
          "estimateFrontier", ...
          "estimateFrontierByReturn", ...
          "estimateFrontierByRisk", ...
          "estimateFrontierLimits", ...
          "estimateMaxSharpeRatio", "estimatePortMoments", ...
          "estimatePortReturn", ...
          "estimatePortRisk", ...
          "estimatePortStd", "estimatePortVaR", ...
          "estimateScenarioMoments", "ewstats", ...
          "exp", "extfield", "fbusdate", "fetch", "fieldnames", "fillts", ...
          "fints", "floatdiscmargin", "floatmargin", "fpctkd", "frac2cur", ...
          "freqnum", "freqstr", "frontcon", "frontier", "fts2ascii", ...
          "fts2mat", "ftsbound", "ftsgui", "ftsinfo", "ftstool", "ftsuniq", ...
          "fvdisc", "fvfix", "fvvar", "fwd2zero", "geom2arith", ...
          "getAssetMoments", "getBounds", ...
          "getBudget", ...
          "getCosts", "getEquality", ...
          "getGroupRatio", "getGroups", ...
          "getInequality", ...
          "getnameidx", "getOneWayTurnover", ...
          "getScenarios", ...
          "hhigh", "highlow", "holdings2weights", ...
          "holidays", "horzcat", "hour", "inforatio", "irr", "isbusday", ...
          "iscompatible", "isfield", "issorted", "kagi", "lagts", ...
          "lbusdate", "leadts", "length", "linebreak", "llow", ...
          "log10", "log2", "lpm", "lweekdate", "m2xdate", "macd", ...
          "maxdrawdown", "medprice", "merge", "minus", "minute", "mirr", ...
          "month", "months", "movavg", "mrdivide", "mtimes", "mvnrfish", ...
          "mvnrmle", "mvnrobj", "mvnrstd", "negvolidx", ...
          "nomrr", "nweekdate", "nyseclosures", "onbalvol", "opprofit", ...
          "payadv", "payodd", "payper", "payuni", "pcalims", "pcgcomp", ...
          "pcglims", "pcpval", "peravg", "periodicreturns", "plotFrontier", ...
          "plus", "pointfig", "portalloc", ...
          "portalpha", "portcons", "Portfolio", "PortfolioCVaR", ...
          "PortfolioMAD", "portopt", "portrand", "portror", "portsim", ...
          "portstats", "portvar", "portvrisk", "posvolidx", "power", ...
          "prbyzero", "prcroc", "prdisc", "priceandvol", "prmat", "prtbill", ...
          "pvfix", "pvtrend", "pvvar", "pyld2zero", "rdivide", "renko", ...
          "resamplets", "ret2tick", "rmfield", "rsindex", "selectreturn", ...
          "setAssetList", "setAssetMoments", ...
          "setBounds", "setBudget", ...
          "setCosts", ...
          "setDefaultConstraints", ...
          "setEquality", ...
          "setGroupRatio", ...
          "setGroups", "setInequality", ...
          "setInitPort", ...
          "setOneWayTurnover", ...
          "setProbabilityLevel", "setScenarios", ...
          "setSolver", ...
          "setTurnover", "sharpe", ...
          "simulateNormalScenariosByData", ...
          "simulateNormalScenariosByMoments", ...
          "smoothts", "sortfts", ...
          "spctkd", "stochosc", "subsasgn", "subsref", "targetreturn", ...
          "taxedrr", "tbilldisc2yield", "tbillprice", "tbillrepo", ...
          "tbillval01", "tbillyield", "tbillyield2disc", "tbl2bond", ...
          "thirdwednesday", "thirtytwo2dec", "tick2ret", "time2date", ...
          "times", "toannual", "todaily", "today", "todecimal", "tomonthly", ...
          "toquarterly", "toquoted", "tosemi", "totalreturnprice", ...
          "toweekly", "tr2bonds", "transprob", "transprobbytotals", ...
          "transprobfromthresholds", "transprobgrouptotals", ...
          "transprobprep", "transprobtothresholds", "tsaccel", "tsmom", ...
          "tsmovavg", "typprice", "ugarch", "ugarchllf", "ugarchpred", ...
          "ugarchsim", "uicalendar", "uminus", "uplus", "vertcat", ...
          "volarea", "volroc", "wclose", "weeknum", "weights2holdings", ...
          "willad", "willpctr", "wrkdydif", "x2mdate", "xirr", "year", ...
          "yeardays", "yearfrac", "ylddisc", "yldmat", "yldtbill", ...
          "zbtprice", "zbtyield", "zero2disc", "zero2fwd", "zero2pyld"}
      txt = check_package (fcn, "financial");

    ## image processing
    case {"activecontour", "adapthisteq", "affine2d", "affine3d", ...
          "analyze75info", "analyze75read", "applycform", "applylut", ...
          "axes2pix", "bestblk", "blockproc", "bwarea", "bwareaopen", ...
          "bwboundaries", "bwconncomp", "bwconvhull", "bwdist", ...
          "bwdistgeodesic", "bweuler", "bwhitmiss", "bwlabel", "bwlabeln", ...
          "bwlookup", "bwmorph", "bwpack", "bwperim", "bwselect", ...
          "bwtraceboundary", "bwulterode", "bwunpack", "checkerboard", ...
          "col2im", "colfilt", "conndef", "convmtx2", "corner", ...
          "cornermetric", "corr2", "cp2tform", "cpcorr", "cpselect", ...
          "cpstruct2pairs", "dct2", "dctmtx", "deconvblind", "deconvlucy", ...
          "deconvreg", "deconvwnr", "decorrstretch", "demosaic", ...
          "dicomanon", "dicomdict", "dicominfo", "dicomlookup", "dicomread", ...
          "dicomuid", "dicomwrite", "edge", "edgetaper", "entropy", ...
          "entropyfilt", "fan2para", "fanbeam", "findbounds", "fitgeotrans", ...
          "fliptform", "freqz2", "fsamp2", "fspecial", "ftrans2", "fwind1", ...
          "fwind2", "getheight", "getimage", "getimagemodel", "getline", ...
          "getneighbors", "getnhood", "getpts", "getrect", "getsequence", ...
          "graycomatrix", "graycoprops", "graydist", "grayslice", ...
          "graythresh", "hdrread", "hdrwrite", "histeq", "hough", ...
          "houghlines", "houghpeaks", "iccfind", "iccread", "iccroot", ...
          "iccwrite", "idct2", "ifanbeam", "im2bw", "im2col", "im2double", ...
          "im2int16", "im2java2d", "im2single", "im2uint16", "im2uint8", ...
          "imabsdiff", "imadd", "imadjust", "ImageAdapter", "imageinfo", ...
          "imapplymatrix", "imapprox", "imattributes", "imbothat", ...
          "imclearborder", "imclose", "imcolormaptool", "imcomplement", ...
          "imcontour", "imcontrast", "imcrop", "imdilate", "imdisplayrange", ...
          "imdistline", "imdivide", "imellipse", "imerode", "imextendedmax", ...
          "imextendedmin", "imfill", "imfilter", "imfindcircles", ...
          "imfreehand", "imfuse", "imgca", "imgcf", "imgetfile", ...
          "imgradient", "imgradientxy", "imhandles", "imhist", ...
          "imhistmatch", "imhmax", "imhmin", "imimposemin", "imlincomb", ...
          "imline", "immagbox", "immovie", "immultiply", "imnoise", ...
          "imopen", "imoverview", "imoverviewpanel", "impixel", ...
          "impixelinfo", "impixelinfoval", "impixelregion", ...
          "impixelregionpanel", "implay", "impoint", "impoly", "improfile", ...
          "impyramid", "imquantize", "imreconstruct", "imrect", "imref2d", ...
          "imref3d", "imregconfig", "imregionalmax", "imregionalmin", ...
          "imregister", "imregtform", "imresize", "imroi", "imrotate", ...
          "imsave", "imscrollpanel", "imsharpen", "imshowpair", ...
          "imsubtract", "imtool", "imtophat", "imtransform", "imwarp", ...
          "interfileinfo", "interfileread", "intlut", "iptaddcallback", ...
          "iptcheckconn", "iptcheckhandle", "iptgetapi", ...
          "iptGetPointerBehavior", "iptgetpref", "ipticondir", ...
          "iptPointerManager", "iptprefs", "iptremovecallback", ...
          "iptSetPointerBehavior", "iptsetpref", "iptwindowalign", "iradon", ...
          "isflat", "isicc", "isrset", "lab2double", "lab2uint16", ...
          "lab2uint8", "label2rgb", "labelmatrix", "makecform", ...
          "makeConstrainToRectFcn", "makehdr", "makelut", "makeresampler", ...
          "maketform", "mat2gray", "mean2", "medfilt2", "montage", ...
          "multithresh", "nitfinfo", "nitfread", "nlfilter", "normxcorr2", ...
          "openrset", "ordfilt2", "otf2psf", "padarray", "para2fan", ...
          "phantom", "poly2mask", "projective2d", "psf2otf", "qtdecomp", ...
          "qtgetblk", "qtsetblk", "radon", "rangefilt", "reflect", ...
          "regionprops", "rgb2gray", "rgb2ycbcr", "roicolor", "roifill", ...
          "roifilt2", "roipoly", "rsetwrite", "std2", "stdfilt", "strel", ...
          "stretchlim", "subimage", "tformarray", "tformfwd", "tforminv", ...
          "tonemap", "translate", "truesize", "viscircles", "warp", ...
          "watershed", "whitepoint", "wiener2", "xyz2double", "xyz2uint16", ...
          "ycbcr2rgb"}
      txt = check_package (fcn, "image");

    ## signal processing
    case {"ac2poly", "ac2rc", "angle", "arburg", "arcov", "armcov", ...
          "aryule", "bandpower", "barthannwin", "besselap", "besself", ...
          "bilinear", "bitrevorder", "blackmanharris", "bohmanwin", ...
          "buffer", "buttap", "butter", "buttord", "cceps", "cconv", ...
          "cell2sos", "cfirpm", "cheb1ap", "cheb1ord", "cheb2ap", ...
          "cheb2ord", "chebwin", "cheby1", "cheby2", "chirp", "convmtx", ...
          "corrmtx", "cpsd", "czt", "db", "db2mag", "db2pow", "dct", ...
          "decimate", "demod", "design", "designmethods", "designopts", ...
          "dfilt", "dftmtx", "digitrevorder", "diric", "downsample", "dpss", ...
          "dpssclear", "dpssdir", "dpssload", "dspdata", "dspfwiz", ...
          "dutycycle", "ellip", "ellipap", "ellipord", "enbw", "equiripple", ...
          "falltime", "fdatool", "fdesign", "filt2block", "filterbuilder", ...
          "filternorm", "filtfilt", "filtic", "filtord", "findpeaks", ...
          "fir1", "fir2", "fircls", "fircls1", "firls", "firpm", "firpmord", ...
          "firrcos", "firtype", "flattopwin", "freqs", "freqsamp", "fvtool", ...
          "fwht", "gauspuls", "gaussdesign", "gaussfir", "gausswin", ...
          "gmonopuls", "goertzel", "grpdelay", "hann", "hilbert", "icceps", ...
          "idct", "ifwht", "impinvar", "impz", "impzlength", "interp", ...
          "intfilt", "invfreqs", "invfreqz", "is2rc", "isallpass", ...
          "islinphase", "ismaxphase", "isminphase", "isstable", "kaiser", ...
          "kaiserord", "kaiserwin", "lar2rc", "latc2tf", "latcfilt", ...
          "levinson", "lp2bp", "lp2bs", "lp2hp", "lp2lp", "lpc", "lsf2poly", ...
          "mag2db", "marcumq", "maxflat", "medfilt1", "midcross", ...
          "modulate", "mscohere", "nuttallwin", "overshoot", "parzenwin", ...
          "pburg", "pcov", "peak2peak", "peak2rms", "peig", "phasedelay", ...
          "phasez", "pmcov", "pmtm", "pmusic", "poly2ac", "poly2lsf", ...
          "poly2rc", "polyscale", "polystab", "pow2db", "prony", ...
          "pulseperiod", "pulsesep", "pulsewidth", "pulstran", "pwelch", ...
          "pyulear", "rc2ac", "rc2is", "rc2lar", "rc2poly", "rceps", ...
          "rcosdesign", "realizemdl", "rectpuls", "rectwin", "resample", ...
          "residuez", "risetime", "rlevinson", "rms", "rooteig", ...
          "rootmusic", "rssq", "sawtooth", "schurrc", "seqperiod", ...
          "setspecs", "settlingtime", "sfdr", "sgolay", "sgolayfilt", ...
          "shiftdata", "sigwin", "sinad", "slewrate", "snr", "sos2cell", ...
          "sos2ss", "sos2tf", "sos2zp", "sosfilt", "spectrogram", ...
          "spectrum", "sptool", "square", "ss2sos", "ss2tf", "ss2zp", ...
          "statelevels", "stepz", "stmcb", "strips", "taylorwin", "tf2latc", ...
          "tf2sos", "tf2ss", "tf2zp", "tf2zpk", "tfestimate", "thd", "toi", ...
          "triang", "tripuls", "tukeywin", "udecode", "uencode", ...
          "undershoot", "unshiftdata", "upfirdn", "upsample", ...
          "validstructures", "vco", "window", "wintool", "wvtool", "xcorr", ...
          "xcorr2", "xcov", "yulewalk", "zerophase", "zp2sos", "zp2ss", ...
          "zp2tf", "zplane"}
      txt = check_package (fcn, "signal");

    ## statistics
    case {"addedvarplot", "addlevels", "addTerms", "adtest", "andrewsplot", ...
          "anova", "anova1", "anova2", "anovan", "ansaribradley", ...
          "aoctool", "bartlett_test", "barttest", "bbdesign", "betacdf", ...
          "betafit", "betainv", "betalike", "betapdf", "betarnd", ...
          "betastat", "binocdf", "binofit", "binoinv", "binopdf", ...
          "binornd", "binostat", "biplot", "bootci", "bootstrp", "boxplot", ...
          "candexch", "candgen", "canoncorr", "capability", "capaplot", ...
          "caseread", "casewrite", "cauchy_cdf", "cauchy_inv", ...
          "cauchy_pdf", "cauchy_rnd", "ccdesign", "cdf", "cdfplot", ...
          "cell2dataset", "chi2cdf", "chi2gof", "chi2inv", "chi2pdf", ...
          "chi2rnd", "chi2stat", "chisquare_test_homogeneity", ...
          "chisquare_test_independence", "cholcov", ...
          "ClassificationBaggedEnsemble", "ClassificationDiscriminant", ...
          "ClassificationEnsemble", "ClassificationKNN", ...
          "ClassificationPartitionedEnsemble", ...
          "ClassificationPartitionedModel", "ClassificationTree", ...
          "classify", "classregtree", "cloglog", "cluster", "clusterdata", ...
          "cmdscale", "coefCI", "coefTest", "combnk", "compact", ...
          "CompactClassificationDiscriminant", ...
          "CompactClassificationEnsemble", "CompactClassificationTree", ...
          "CompactRegressionEnsemble", "CompactRegressionTree", ...
          "CompactTreeBagger", "compare", "confusionmat", "controlchart", ...
          "controlrules", "cophenet", "copulacdf", "copulafit", ...
          "copulaparam", "copulapdf", "copularnd", "copulastat", ...
          "cordexch", "corrcov", "cor_test", "covarianceParameters", ...
          "coxphfit", "createns", "crosstab", "crossval", "cvpartition", ...
          "datasample", "dataset", "dataset2cell", "dataset2struct", ...
          "dataset2table", "datasetfun", "daugment", "dcovary", ...
          "dendrogram", "designMatrix", "devianceTest", "dfittool", ...
          "disttool", "droplevels", "dummyvar", "dwtest", "ecdf", ...
          "ecdfhist", "evalclusters", "evcdf", "evfit", "evinv", "evlike", ...
          "evpdf", "evrnd", "evstat", "ExhaustiveSearcher", "expcdf", ...
          "expfit", "expinv", "explike", "export", "exppdf", "exprnd", ...
          "expstat", "factoran", "fcdf", "ff2n", "finv", "fitdist", ...
          "fitensemble", "fitglm", "fitlm", "fitlme", "fitlmematrix", ...
          "fitnlm", "fitted", "fixedEffects", "fpdf", "fracfact", ...
          "fracfactgen", "friedman", "frnd", "fsurfht", "fullfact", ...
          "f_test_regression", "gagerr", "gamcdf", "gamfit", "gaminv", ...
          "gamlike", "gampdf", "gamrnd", "gamstat", ...
          "GeneralizedLinearModel", "geocdf", "geoinv", "geomean", ...
          "geopdf", "geornd", "geostat", "getlabels", "getlevels", ...
          "gevcdf", "gevfit", "gevinv", "gevlike", "gevpdf", "gevrnd", ...
          "gevstat", "gline", "glmfit", "glmval", "glyphplot", ...
          "gmdistribution", "gname", "gpcdf", "gpfit", "gpinv", "gplike", ...
          "gplotmatrix", "gppdf", "gprnd", "gpstat", "grp2idx", "grpstats", ...
          "gscatter", "haltonset", "harmmean", "hist3", "histfit", ...
          "hmmdecode", "hmmestimate", "hmmgenerate", "hmmtrain", ...
          "hmmviterbi", "hotelling_test", "hotelling_test_2", "hougen", ...
          "hygecdf", "hygeinv", "hygepdf", "hygernd", "hygestat", "icdf", ...
          "inconsistent", "interactionplot", "invpred", "islevel", ...
          "ismissing", "isundefined", "iwishrnd", "jackknife", "jbtest", ...
          "johnsrnd", "join", "KDTreeSearcher", "kmeans", "knnsearch", ...
          "kolmogorov_smirnov_cdf", "kolmogorov_smirnov_test", ...
          "kolmogorov_smirnov_test_2", "kruskalwallis", ...
          "kruskal_wallis_test", "ksdensity", "kstest", "kstest2", ...
          "labels", "laplace_cdf", "laplace_inv", "laplace_pdf", ...
          "laplace_rnd", "lasso", "lassoglm", "lassoPlot", "levelcounts", ...
          "leverage", "lhsdesign", "lhsnorm", "lillietest", ...
          "LinearMixedModel", "LinearModel", "linhyptest", "linkage", ...
          "logistic_cdf", "logistic_inv", "logistic_pdf", ...
          "logistic_regression", "logistic_rnd", "logit", "logncdf", ...
          "lognfit", "logninv", "lognlike", "lognpdf", "lognrnd", ...
          "lognstat", "lsline", "mad", "mahal", "maineffectsplot", ...
          "makedist", "manova", "manova1", "manovacluster", "mat2dataset", ...
          "mcnemar_test", "mdscale", "mergelevels", "mhsample", "mle", ...
          "mlecov", "mnpdf", "mnrfit", "mnrnd", "mnrval", "multcompare", ...
          "multivarichart", "mvncdf", "mvnpdf", "mvnrnd", "mvregress", ...
          "mvregresslike", "mvtcdf", "mvtpdf", "mvtrnd", "NaiveBayes", ...
          "nancov", "nanmax", "nanmean", "nanmedian", "nanmin", "nanstd", ...
          "nansum", "nanvar", "nbincdf", "nbinfit", "nbininv", "nbinpdf", ...
          "nbinrnd", "nbinstat", "ncfcdf", "ncfinv", "ncfpdf", "ncfrnd", ...
          "ncfstat", "nctcdf", "nctinv", "nctpdf", "nctrnd", "nctstat", ...
          "ncx2cdf", "ncx2inv", "ncx2pdf", "ncx2rnd", "ncx2stat", ...
          "negloglik", "nlinfit", "nlintool", "nlmefit", "nlmefitsa", ...
          "nlparci", "nlpredci", "nnmf", "nominal", "NonLinearModel", ...
          "normcdf", "normfit", "norminv", "normlike", "normpdf", ...
          "normplot", "normrnd", "normspec", "normstat", ...
          "optimalleaforder", "ordinal", "parallelcoords", "paramci", ...
          "paretotails", "partialcorr", "partialcorri", "pca", "pcacov", ...
          "pcares", "pdf", "pdist", "pdist2", "pearsrnd", "perfcurve", ...
          "plotAdded", "plotAdjustedResponse", "plotDiagnostics", ...
          "plotEffects", "plotInteraction", "plotResiduals", "plotSlice", ...
          "plsregress", "poisscdf", "poissfit", "poissinv", "poisspdf", ...
          "poissrnd", "poisstat", "polyconf", "polytool", "ppca", ...
          "prctile", "predict", "princomp", "ProbDistUnivKernel", ...
          "ProbDistUnivParam", "probit", "probplot", "procrustes", ...
          "proflik", "prop_test_2", "qqplot", "qrandset", "qrandstream", ...
          "random", "randomEffects", "randsample", "randtool", ...
          "rangesearch", "ranksum", "raylcdf", "raylfit", "raylinv", ...
          "raylpdf", "raylrnd", "raylstat", "rcoplot", "refcurve", ...
          "refline", "regress", "RegressionBaggedEnsemble", ...
          "RegressionEnsemble", "RegressionPartitionedEnsemble", ...
          "RegressionPartitionedModel", "RegressionTree", "regstats", ...
          "relieff", "removeTerms", "residuals", "response", "ridge", ...
          "robustdemo", "robustfit", "rotatefactors", "rowexch", "rsmdemo", ...
          "rstool", "runstest", "run_test", "sampsizepwr", "scatterhist", ...
          "sequentialfs", "setlabels", "signrank", "signtest", "sign_test", ...
          "silhouette", "slicesample", "sobolset", "squareform", "statget", ...
          "statset", "stdnormal_cdf", "stdnormal_inv", "stdnormal_pdf", ...
          "stdnormal_rnd", "step", "stepwise", "stepwisefit", ...
          "stepwiseglm", "stepwiselm", "struct2dataset", "surfht", ...
          "svmclassify", "svmtrain", "table2dataset", "tabulate", ...
          "tblread", "tblwrite", "tcdf", "tdfread", "tiedrank", "tinv", ...
          "tpdf", "TreeBagger", "trimmean", "trnd", "truncate", "tstat", ...
          "ttest", "ttest2", "t_test", "t_test_2", "t_test_regression", ...
          "unidcdf", "unidinv", "unidpdf", "unidrnd", "unidstat", ...
          "unifcdf", "unifinv", "unifit", "unifpdf", "unifrnd", "unifstat", ...
          "u_test", "vartest", "vartest2", "vartestn", "var_test", ...
          "wblcdf", "wblfit", "wblinv", "wbllike", "wblpdf", "wblplot", ...
          "wblrnd", "wblstat", "welch_test", "wienrnd", "wilcoxon_test", ...
          "wishrnd", "x2fx", "xlsread", "xptread", "ztest", "z_test", ...
          "z_test_2"}
      txt = check_package (fcn, "statistics");

    ## symbolic
    case {"argnames", "bernoulli", "catalan", "charpoly", "chebyshevT", ...
          "chebyshevU", "children", "coshint", "cosint", "dawson", ...
          "digits", "dilog", "dirac", "divisors", "dsolve", "ei", ...
          "ellipticCE", "ellipticCK", "ellipticCPi", "ellipticE", ...
          "ellipticF", "ellipticK", "ellipticPi", "euler", "eulergamma", ...
          "expand", "finverse", "formula", "fresnelc", "fresnels", ...
          "funtool", "gegenbauerC", "harmonic", "heaviside", "hermiteH", ...
          "hypergeom", "igamma", "jacobiP", "kroneckerDelta", "kummerU", ...
          "laguerreL", "legendreP", "logint", "minpoly", "pochhammer", ...
          "poles", "poly2sym", "polylog", "pretty", "quorem", ...
          "rectangularPulse", "signIm", "simplify", "sinhint", "sinint", ...
          "solve", "ssinint", "sym", "sym2poly", "symfun", "sympref", ...
          "syms", "symvar", "triangularPulse", "vpa", "vpasolve", ...
          "whittakerM", "whittakerW", "zeta"}
      classes = {"sym", "symfun"};
      txt = check_package (fcn, "symbolic", classes);

    ## optimization
    case {"bintprog", "color", "fgoalattain", "fmincon", "fminimax", ...
          "fminsearch", "fseminf", "fzmult", "gangstr", "ktrlink", ...
          "linprog", "lsqcurvefit", "lsqlin", "lsqnonlin", "optimoptions", ...
          "optimtool", "quadprog"}
      txt = check_package (fcn, "optim");

    otherwise
      if (ismember (fcn, missing_functions ()))
        txt = ["The '" fcn "' function is not yet implemented in Octave."];
      else
        is_matlab_function = false;
        txt = "";
      endif
  endswitch

  if (is_matlab_function)
    txt = [txt, "\n\n@noindent\nPlease read ", ...
           "@url{https://www.octave.org/missing.html} to learn how ", ...
           "you can contribute missing functionality."];
    txt = __makeinfo__ (txt);
  endif

  if (nargout == 0)
    error ("Octave:missing-function", "%s\n", txt);
  endif

endfunction

function txt = check_package (fcn, name, classes)

  if (nargin < 3)
    classes = {};
  endif

  txt = sprintf ("The '%s' function belongs to the %s package from Octave Forge",
                 fcn, name);

  [~, status] = pkg ("describe", name);
  switch (tolower (status{1}))
    case "loaded",
      for i = 1:length (classes)
        cls = classes{i};
        try
          meths = methods (cls);
        catch
          meths = {};
        end_try_catch
        if (any (strcmp (fcn, meths)))
          txt = sprintf (["'%s' is a method of class '%s'; it must be ", ...
                          "called with a '%s' argument (see 'help @@%s/%s')."],
                         fcn, cls, cls, cls, fcn);
          return
        endif
      endfor
      txt = sprintf ("%s but has not yet been implemented.", txt);
    case "not loaded",
      txt = sprintf (["%s which you have installed but not loaded.  To ", ...
                      "load the package, run 'pkg load %s' from the ", ...
                      "Octave prompt."], txt, name);
    otherwise
      ## this includes "not installed" and anything else if pkg changes
      ## the output of describe
      txt = sprintf ("%s which seems to not be installed in your system.", txt);
  endswitch

endfunction

function rlist = missing_functions ()
  persistent list = {
  "addboundary",
  "addcats",
  "addCause",
  "addedge",
  "addmulti",
  "addnode",
  "addpoints",
  "addRequired",
  "addsample",
  "addsampletocollection",
  "addts",
  "adjacency",
  "align",
  "alim",
  "alpha",
  "alphamap",
  "alphaShape",
  "alphaSpectrum",
  "alphaTriangulation",
  "animatedline",
  "animatedLine",
  "append",
  "array2table",
  "array2timetable",
  "bar3",
  "bar3h",
  "barycentricToCartesian",
  "bctree",
  "bench",
  "between",
  "bfsearch",
  "bicgstabl",
  "biconncomp",
  "binary",
  "binscatter",
  "bitnot",
  "boundary",
  "boundaryFacets",
  "boundingbox",
  "brush",
  "builddocsearchdb",
  "bvp4c",
  "bvp5c",
  "bvpget",
  "bvpinit",
  "bvpset",
  "bvpxtend",
  "caldays",
  "caldiff",
  "calendarDuration",
  "calllib",
  "callSoapService",
  "calmonths",
  "calquarters",
  "calweeks",
  "calyears",
  "camdolly",
  "cameratoolbar",
  "campan",
  "camproj",
  "cartesianToBarycentric",
  "cartToBary",
  "categorical",
  "categories",
  "cdf2rdf",
  "cdfepoch",
  "cdfinfo",
  "cdflib",
  "cdfread",
  "cdfwrite",
  "cell2table",
  "cellplot",
  "centrality",
  "centroid",
  "checkcode",
  "checkin",
  "checkout",
  "circumcenter",
  "circumcenters",
  "classUnderlying",
  "clearAllMemoizedCaches",
  "clearCache",
  "clearpoints",
  "clipboard",
  "clone",
  "cmopts",
  "collapse",
  "colordef",
  "colormapeditor",
  "compose",
  "condensation",
  "coneplot",
  "conncomp",
  "contains",
  "contourslice",
  "convertCharsToStrings",
  "convertStringsToChars",
  "convexHull",
  "copyHDU",
  "count",
  "countcats",
  "countEachLabel",
  "createClassFromWsdl",
  "createFile",
  "createImg",
  "createSoapMessage",
  "createTbl",
  "criticalAlpha",
  "customverctrl",
  "daqread",
  "datacursormode",
  "datastore",
  "datatipinfo",
  "dateshift",
  "datetime",
  "day",
  "days",
  "dbmex",
  "dde23",
  "ddeget",
  "ddensd",
  "ddesd",
  "ddeset",
  "decomposition",
  "degree",
  "delaunayTriangulation",
  "deleteCol",
  "deleteFile",
  "deleteHDU",
  "deleteKey",
  "deleteproperty",
  "deleteRecord",
  "deleteRows",
  "delevent",
  "DelimitedTextImportOptions",
  "delsample",
  "delsamplefromcollection",
  "depdir",
  "depfun",
  "details",
  "detectImportOptions",
  "deval",
  "dfsearch",
  "digraph",
  "discretize",
  "dissect",
  "distances",
  "dither",
  "docsearch",
  "dragrect",
  "duration",
  "dynamicprops",
  "echodemo",
  "edgeAttachments",
  "edges",
  "empty",
  "enableservice",
  "enumeration",
  "eraseBetween",
  "eventlisteners",
  "events",
  "exceltime",
  "Execute",
  "export2wsdlg",
  "exportsetupdlg",
  "extractAfter",
  "extractBefore",
  "extractBetween",
  "faceNormal",
  "faceNormals",
  "fcontour",
  "featureEdges",
  "Feval",
  "fewerbins",
  "figurepalette",
  "FileDatastore",
  "filemarker",
  "fileMode",
  "fileName",
  "fill3",
  "fillmissing",
  "filloutliers",
  "fimplicit",
  "fimplicit3",
  "findedge",
  "findgroups",
  "findnode",
  "findprop",
  "fitsdisp",
  "fitsinfo",
  "fitsread",
  "fitswrite",
  "FixedWidthImportOptions",
  "flipedge",
  "flow",
  "fmesh",
  "fplot3",
  "freeBoundary",
  "freqspace",
  "fsurf",
  "FunctionTestCase",
  "functiontests",
  "funm",
  "gather",
  "gcmr",
  "geobubble",
  "geolimits",
  "getabstime",
  "getAColParms",
  "getaudiodata",
  "getBColParms",
  "GetCharArray",
  "getColName",
  "getColType",
  "getConstantValue",
  "getdatasamples",
  "getdatasamplesize",
  "getEqColType",
  "getFileFormats",
  "GetFullMatrix",
  "getHdrSpace",
  "getHDUnum",
  "getHDUtype",
  "getImgSize",
  "getImgType",
  "getinterpmethod",
  "getnext",
  "getNumCols",
  "getNumHDUs",
  "getNumInputs",
  "getNumOutputs",
  "getNumRows",
  "getOpenFiles",
  "getpixelposition",
  "getpoints",
  "getProfiles",
  "getqualitydesc",
  "getReport",
  "getsamples",
  "getsampleusingtime",
  "gettimeseriesnames",
  "gettsafteratevent",
  "gettsafterevent",
  "gettsatevent",
  "gettsbeforeatevent",
  "gettsbeforeevent",
  "gettsbetweenevents",
  "GetVariable",
  "getvaropts",
  "getVersion",
  "GetWorkspaceData",
  "gobjects",
  "graph",
  "GraphPlot",
  "graymon",
  "griddedInterpolant",
  "guide",
  "h5create",
  "h5disp",
  "h5info",
  "h5read",
  "h5readatt",
  "h5write",
  "h5writeatt",
  "handle",
  "hasdata",
  "hasFrame",
  "hasnext",
  "hdf5info",
  "hdf5read",
  "hdf5write",
  "hdfan",
  "hdfdf24",
  "hdfdfr8",
  "hdfh",
  "hdfhd",
  "hdfhe",
  "hdfhx",
  "hdfinfo",
  "hdfml",
  "hdfpt",
  "hdfread",
  "hdftool",
  "hdfv",
  "hdfvf",
  "hdfvh",
  "hdfvs",
  "head",
  "heatmap",
  "height",
  "helpbrowser",
  "helpdesk",
  "helpwin",
  "hgexport",
  "hgsetget",
  "highlight",
  "histcounts",
  "histcounts2",
  "histogram",
  "histogram2",
  "hms",
  "holes",
  "hour",
  "hours",
  "idealfilter",
  "im2java",
  "ImageDatastore",
  "imapprox",
  "imgCompress",
  "import",
  "imresize",
  "incenter",
  "incenters",
  "incidence",
  "indegree",
  "inmem",
  "innerjoin",
  "inOutStatus",
  "insertAfter",
  "insertATbl",
  "insertBefore",
  "insertBTbl",
  "insertCol",
  "insertImg",
  "insertRows",
  "inShape",
  "inspect",
  "instrcallback",
  "instrfind",
  "instrfindall",
  "interfaces",
  "interp1q",
  "interpstreamspeed",
  "invoke",
  "isaUnderlying",
  "isbetween",
  "iscalendarduration",
  "iscategorical",
  "iscategory",
  "ischange",
  "iscom",
  "isCompressedImg",
  "isConnected",
  "isdag",
  "isdatetime",
  "isdst",
  "isduration",
  "isEdge",
  "isenum",
  "isevent",
  "ishole",
  "isIllConditioned",
  "isinterface",
  "isinterior",
  "isInterior",
  "isisomorphic",
  "islocalmax",
  "islocalmin",
  "isLocked",
  "ismembertol",
  "ismissing",
  "isnat",
  "isomorphism",
  "isordinal",
  "isoutlier",
  "isprotected",
  "isregular",
  "issimplified",
  "issortedrows",
  "isStringScalar",
  "istable",
  "istall",
  "istimetable",
  "isundefined",
  "isvalid",
  "isweekend",
  "javaMethodEDT",
  "javaObjectEDT",
  "join",
  "jsondecode",
  "jsonencode",
  "juliandate",
  "labeledge",
  "labelnode",
  "lag",
  "laplacian",
  "last",
  "layout",
  "ldl",
  "libfunctions",
  "libfunctionsview",
  "libisloaded",
  "libpointer",
  "libstruct",
  "linkdata",
  "listfonts",
  "loadlibrary",
  "lsqminnorm",
  "lsqr",
  "makehgtform",
  "mapreduce",
  "mapreducer",
  "matfile",
  "maxflow",
  "MaximizeCommandWindow",
  "maxk",
  "maxNumCompThreads",
  "memmapfile",
  "memoize",
  "MemoizedFunction",
  "memory",
  "mergecats",
  "meta.abstractDetails",
  "meta.class.fromName",
  "meta.DynamicProperty",
  "meta.EnumeratedValue",
  "meta.MetaData",
  "meta.package.fromName",
  "meta.package.getAllPackages",
  "methodsview",
  "MException",
  "milliseconds",
  "MinimizeCommandWindow",
  "mink",
  "minres",
  "minspantree",
  "minute",
  "minutes",
  "missing",
  "mlint",
  "mlintrpt",
  "mmfileinfo",
  "month",
  "morebins",
  "movAbsHDU",
  "move",
  "movie2avi",
  "movNamHDU",
  "movRelHDU",
  "mput",
  "multibandread",
  "multibandwrite",
  "NaT",
  "nccreate",
  "ncdisp",
  "ncinfo",
  "ncread",
  "ncreadatt",
  "ncwrite",
  "ncwriteatt",
  "ncwriteschema",
  "nearest",
  "nearestNeighbor",
  "neighbors",
  "netcdf",
  "noanimate",
  "notebook",
  "notify",
  "nsidedpoly",
  "num2ruler",
  "numArgumentsFromSubscript",
  "numboundaries",
  "numedges",
  "numnodes",
  "numpartitions",
  "numRegions",
  "numsides",
  "ode113",
  "ode23t",
  "ode23tb",
  "odextend",
  "openFile",
  "opengl",
  "ordqz",
  "outdegree",
  "outerjoin",
  "pad",
  "pagesetupdlg",
  "parse",
  "parseSoapResponse",
  "partition",
  "parula",
  "path2rc",
  "pathtool",
  "pcode",
  "pdepe",
  "pdeval",
  "perimeter",
  "plotbrowser",
  "plotedit",
  "plottools",
  "pointLocation",
  "polaraxes",
  "polarhistogram",
  "polarplot",
  "polarscatter",
  "polybuffer",
  "polyshape",
  "posixtime",
  "predecessors",
  "preview",
  "printdlg",
  "printopt",
  "printpreview",
  "profsave",
  "propedit",
  "properties",
  "propertyeditor",
  "PutCharArray",
  "PutFullMatrix",
  "PutWorkspaceData",
  "pyargs",
  "pyversion",
  "quarter",
  "Quit",
  "RandStream",
  "rbbox",
  "readall",
  "readasync",
  "readATblHdr",
  "readBTblHdr",
  "readCard",
  "readCol",
  "readFrame",
  "readimage",
  "readImg",
  "readKey",
  "readKeyCmplx",
  "readKeyDbl",
  "readKeyLongLong",
  "readKeyLongStr",
  "readKeyUnit",
  "readRecord",
  "readtable",
  "recordblocking",
  "regions",
  "registerevent",
  "regmatlabserver",
  "release",
  "Remove",
  "RemoveAll",
  "removecats",
  "removets",
  "renamecats",
  "reordercats",
  "reordernodes",
  "replace",
  "replaceBetween",
  "resample",
  "rescale",
  "retime",
  "reverse",
  "rgb2gray",
  "rlim",
  "rmboundary",
  "rmedge",
  "rmholes",
  "rmmissing",
  "rmnode",
  "rmslivers",
  "rng",
  "rowfun",
  "rtickangle",
  "rtickformat",
  "rticklabels",
  "ruler2num",
  "runperf",
  "scale",
  "scatteredInterpolant",
  "scroll",
  "second",
  "seconds",
  "selectmoveresize",
  "sendmail",
  "serial",
  "serialbreak",
  "seriallist",
  "setabstime",
  "setBscale",
  "setcats",
  "setCompressionType",
  "setHCompScale",
  "setHCompSmooth",
  "setinterpmethod",
  "setpixelposition",
  "setstr",
  "setTileDim",
  "settimeseriesnames",
  "setTscale",
  "setuniformtime",
  "setvaropts",
  "setvartype",
  "sheetnames",
  "shortestpath",
  "shortestpathtree",
  "showplottool",
  "shuffle",
  "simplify",
  "smoothdata",
  "snapnow",
  "sortboundaries",
  "sortregions",
  "sortx",
  "sorty",
  "split",
  "splitapply",
  "splitEachLabel",
  "splitlines",
  "SpreadsheetDatastore",
  "SpreadsheetImportOptions",
  "ss2tf",
  "stack",
  "standardizeMissing",
  "stats",
  "step",
  "stopasync",
  "str2mat",
  "streamparticles",
  "streamribbon",
  "streamslice",
  "string",
  "strings",
  "strip",
  "strlength",
  "struct2table",
  "subgraph",
  "subtract",
  "subvolume",
  "successors",
  "summary",
  "superclasses",
  "support",
  "supportPackageInstaller",
  "surf2patch",
  "surfaceArea",
  "symmlq",
  "synchronize",
  "syntax",
  "table",
  "table2array",
  "table2cell",
  "table2struct",
  "table2timetable",
  "TabularTextDatastore",
  "tail",
  "tall",
  "TallDatastore",
  "tallrng",
  "targetupdater",
  "tcpclient",
  "Test",
  "TestResult",
  "testsuite",
  "texlabel",
  "textwrap",
  "tfqmr",
  "thetalim",
  "thetatickformat",
  "thetaticklabels",
  "throw",
  "throwAsCaller",
  "Tiff",
  "timeit",
  "timeofday",
  "timer",
  "timerange",
  "timeseries",
  "timetable",
  "timetable2table",
  "timezones",
  "todatenum",
  "toolboxdir",
  "topkrows",
  "toposort",
  "transclosure",
  "translate",
  "transreduction",
  "triangulation",
  "TriRep",
  "TriScatteredInterp",
  "tscollection",
  "tzoffset",
  "uialert",
  "uiaxes",
  "uibutton",
  "uicheckbox",
  "uiconfirm",
  "uidropdown",
  "uieditfield",
  "uifigure",
  "uigauge",
  "uigetpref",
  "uiimport",
  "uiknob",
  "uilabel",
  "uilamp",
  "uilistbox",
  "uiopen",
  "uiradiobutton",
  "uisave",
  "uisetcolor",
  "uisetpref",
  "uislider",
  "uispinner",
  "uistack",
  "uiswitch",
  "uitab",
  "uitabgroup",
  "uitextarea",
  "uitogglebutton",
  "uitree",
  "uitreenode",
  "undocheckout",
  "uniquetol",
  "unloadlibrary",
  "unmesh",
  "unregisterallevents",
  "unregisterevent",
  "unstack",
  "upgradePreviouslyInstalledSupportPackages",
  "userpath",
  "ValueIterator",
  "varfun",
  "vartype",
  "verctrl",
  "vertexAttachments",
  "vertexNormal",
  "VideoReader",
  "VideoWriter",
  "viewmtx",
  "visdiff",
  "volume",
  "volumebounds",
  "voronoiDiagram",
  "websave",
  "week",
  "whatsnew",
  "width",
  "winopen",
  "withtol",
  "wordcloud",
  "write",
  "writeChecksum",
  "writeCol",
  "writeComment",
  "writeDate",
  "writeHistory",
  "writeImg",
  "writeKey",
  "writeKeyUnit",
  "writetable",
  "writeVideo",
  "xmlread",
  "xmlwrite",
  "xslt",
  "xtickangle",
  "xtickformat",
  "year",
  "years",
  "ymd",
  "ytickangle",
  "ytickformat",
  "yyaxis",
  "yyyymmdd",
  "ztickangle",
  "ztickformat",
  };

  rlist = list;
endfunction


%!test
%! str = __unimplemented__ ("no_name_function");
%! assert (isempty (str));

%!test
%! str = __unimplemented__ ("matlabrc");
%! assert (str(1:71), "matlabrc is not implemented.  Octave uses the file '.octaverc' instead.");

%!test
%! str = __unimplemented__ ("MException");
%! assert (str(1:58), "The 'MException' function is not yet implemented in Octave");
