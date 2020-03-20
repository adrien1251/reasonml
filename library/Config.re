let root = "/tmp/irmin/test";

let init = () => {
  let _ = Sys.command(Printf.sprintf("rm -rf %s", root));
  let _ = Sys.command(Printf.sprintf("mkdir -p %s", root));
  ();
};

/* Install the FS listener. */
let () = Irmin_unix.set_listen_dir_hook();
