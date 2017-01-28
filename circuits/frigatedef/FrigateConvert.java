

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;

/**
 * Created by Arash Afshar.
 *
 * Converts output of frigate to input of scapi
 */
public class FrigateConvert {
    // scapi intput format:
//    from cryptobiu/scapi/blob/master/src/java/edu/biu/scapi/circuits/circuit/BooleanCircuit.java :
//    * The File first lists the number of {@code Gate}s, then the number of parties. <p>
//    * Then for each party: party number, the number of inputs for that party, and following there is a list of indices of each of these input {@code Wire}s.<p>
//    * Next it lists the number of output {@code Wire}s followed by the index of each of these {@code Wires}. <p>
//    * Then for each gate, we have the following: number of inputWires, number of OutputWires inputWireIndices OutputWireIndices and the gate's truth Table (as a 0-1 string).<P>
//            * example file: 1 2 1 1 1 2 1 2 1 3 2 1 1 2 3 0001<p>

    public static void main(String[] args) throws IOException {
        String prefix = args[0] + "/";//"src/main/resources/circuits/";
        String name = args[1]; //"execute";
        String frigate_out_path = prefix + name + ".out";
        String scapi_file_path = prefix + name + ".circ";
//        String frigate_out_path = args[0];
//        String scapi_file_path = args[1];
        new FrigateConvert(frigate_out_path, scapi_file_path);
    }

    public static String num_formatter(int num, int max_len){
        String str_rep = Integer.toBinaryString(num);
        int padding = max_len - str_rep.length();
        assert padding >= 0;
        String result;
        if (padding == 0){
            result = String.format("%s", str_rep);
        }else {
            result = String.format("%0" + padding + "d%s", 0, str_rep);
        }
        return result;
    }

    public FrigateConvert(String frigate_out_path, String scapi_file_path) throws IOException {
        ArrayList<String> p1_inputs = new ArrayList<>();
        ArrayList<String> p2_inputs = new ArrayList<>();
        ArrayList<String> outputs = new ArrayList<>();
        ArrayList<Gate> gates = new ArrayList<>();
        int number_of_gates;

        String copy_6 = "0011";


        Scanner scanner = new Scanner(new File(frigate_out_path));
        while (scanner.hasNext()) {
            String line = scanner.nextLine();
            if (line.contains("IN")){
                String party = line.split(" ")[2];
                String wire = line.split(" ")[1];
                if (party.equals("1")) {
                    p1_inputs.add(wire);
                }else {
                    p2_inputs.add(wire);
                }
            }else if (line.contains("OUT")) {
                // we assume evaluator always has the inputs
                String party = line.split(" ")[2];
                String wire = line.split(" ")[1];
                outputs.add(wire);
            }else if (line.contains("copy(6)")){
                String truth = copy_6;
                String tokens[] = line.split(" ");
                Gate gate = new Gate(new String []{tokens[2], tokens[3]}, tokens[1], truth);
                gates.add(gate);
            }else if (line.contains("copy")){
                System.err.println("Not handled");
            }else{
                String tokens[] = line.split(" ");
                Gate gate = new Gate(new String []{tokens[2], tokens[3]}, tokens[1], Integer.parseInt(tokens[0]));
                gates.add(gate);
            }

        }
        int p1_input_count = p1_inputs.size();
        int p2_input_count = p2_inputs.size();
        int output_count = outputs.size();
        number_of_gates = gates.size();

        //----------------- write to file
        FileOutputStream out = new FileOutputStream(scapi_file_path);
        out.write((String.valueOf(number_of_gates) + "\n\n").getBytes());
        int number_of_parties = 2;
        out.write((String.valueOf(number_of_parties) + "\n\n").getBytes());
        // p1
        out.write(("1 " + String.valueOf(p1_input_count) + "\n").getBytes());
        for (String inp_wire : p1_inputs) {
            out.write((inp_wire + "\n").getBytes());
        }
        out.write(("\n").getBytes());
        // p2
        out.write(("2 " + String.valueOf(p2_input_count) + "\n").getBytes());
        for (String inp_wire : p2_inputs) {
            out.write((inp_wire + "\n").getBytes());
        }
        out.write(("\n").getBytes());
        // output
        out.write((String.valueOf(output_count) + "\n").getBytes());
        for (String out_wire : outputs) {
            out.write((out_wire + "\n").getBytes());
        }
        out.write(("\n").getBytes());
        // gates
        for(Gate gate : gates){
            out.write((gate + "\n").getBytes());
        }

        out.close();
    }

    class Gate{
        public String[] inputs;
        public String output;
        public String type;
        public Gate(final String[] inputs, String output, String type){
            this.inputs = inputs;
            this.output = output;
            this.type = type;
        }
        public Gate(final String[] inputs, String output, int type){
            this.inputs = inputs;
            this.output = output;
            this.type = truth_table(type);
        }
        private String truth_table(int truth_value){
            String tmp = num_formatter(truth_value, 4);
            return new StringBuilder(tmp).reverse().toString();
        }

        @Override
        public String toString() {
            String rep = "";

            String inp_out_count = String.valueOf(inputs.length) + " 1 ";
            rep = inp_out_count;
            for (String inp : inputs){
                rep += inp + " ";
            }

            rep += output + " ";
            rep += type;
            return rep;
        }
    }

}
