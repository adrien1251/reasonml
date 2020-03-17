open Lwt.Infix;

module Car = {
  type color =
    | Black
    | White
    | Other(string);

  type t = {
    license: string,
    year: int32,
    make_and_model: (string, string),
    color,
    owner: string,
  };

  let color =
    Irmin.Type.(
      variant("color", (black, white, other) =>
        fun
        | Black => black
        | White => white
        | Other(color) => other(color)
      )
      |~ case0("Black", Black)
      |~ case0("White", White)
      |~ case1("Other", string, s => Other(s))
      |> sealv
    );

  let t =
    Irmin.Type.(
      record("car", (license, year, make_and_model, color, owner) =>
        {license, year, make_and_model, color, owner}
      )
      |+ field("license", string, t => t.license)
      |+ field("year", int32, t => t.year)
      |+ field("make_and_model", pair(string, string), t => t.make_and_model)
      |+ field("color", color, t => t.color)
      |+ field("owner", string, t => t.owner)
      |> sealr
    );

  let merge = Irmin.Merge.(option(idempotent(t)));
};

module Store = Irmin_unix.Git.Mem.KV(Car);

module Custom_types = {
  module Defaults = Irmin_graphql.Server.Default_types(Store);
  module Key = Defaults.Key;
  module Metadata = Defaults.Metadata;
  module Hash = Defaults.Hash;
  module Branch = Defaults.Branch;

  module Contents = {
    open Graphql_lwt;

    let color_values =
      Schema.[
        enum_value("BLACK", ~value=Car.Black),
        enum_value("WHITE", ~value=Car.White),
      ];

    let schema_typ =
      Schema.(
        obj("Car", ~fields=_ =>
          [
            field(
              "license", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              car.Car.license
            ),
            field("year", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              car.Car.license
            ),
            field("make", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              fst(car.Car.make_and_model)
            ),
            field(
              "model", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              snd(car.Car.make_and_model)
            ),
            field(
              "color", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              car.Car.license
            ),
            field(
              "owner", ~typ=non_null(string), ~args=[], ~resolve=(_, car) =>
              car.Car.owner
            ),
          ]
        )
      );

    let color = Schema.Arg.enum("Color", ~values=color_values);

    let arg_typ =
      Schema.Arg.(
        obj(
          "CarInput",
          ~fields=[
            arg("license", ~typ=non_null(string)),
            arg("year", ~typ=non_null(int)),
            arg("make", ~typ=non_null(string)),
            arg("model", ~typ=non_null(string)),
            arg("color", ~typ=non_null(color)),
            arg("owner", ~typ=non_null(string)),
          ],
          ~coerce=(license, year, make, model, color, owner) =>
          {
            Car.license,
            year: Int32.of_int(year),
            make_and_model: (make, model),
            color,
            owner,
          }
        )
      );
  };
};

module Remote = {
  let remote = Some(Store.remote);
};

module Server =
  Irmin_unix.Graphql.Server.Make_ext(Store, Remote, Custom_types);

let main = () => {
  Config.init();
  let config = Irmin_git.config(Config.root);
  Store.Repo.v(config)
  >>= (
    repo => {
      let server = Server.v(repo);
      let src = "localhost";
      let port = 9876;
      Conduit_lwt_unix.init(~src, ())
      >>= (
        ctx => {
          let ctx = Cohttp_lwt_unix.Net.init(~ctx, ());
          let on_exn = exn =>
            Printf.printf("on_exn: %s", Printexc.to_string(exn));
          Printf.printf(
            "Visit GraphiQL @ http://%s:%d/graphql\n%!",
            src,
            port,
          );
          Cohttp_lwt_unix.Server.create(
            ~on_exn,
            ~ctx,
            ~mode=`TCP(`Port(port)),
            server,
          );
        }
      );
    }
  );
};

let () = Lwt_main.run(main());
