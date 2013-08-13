import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.Scanner;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import net.sf.json.JSONArray;
import net.sf.json.JSONException;
import net.sf.json.JSONObject;


public class TransferServlet extends HttpServlet {

	
	/**
	 * Constructor of the object.
	 */
	public TransferServlet() {
		super();
	}

	/**
	 * Destruction of the servlet. <br>
	 */
	public void destroy() {
		super.destroy(); // Just puts "destroy" string in log
		// Put your code here
	}

	/**
	 * The doGet method of the servlet. <br>
	 *
	 * This method is called when a form has its tag value method equals to get.
	 * 
	 * @param request the request send by the client to the server
	 * @param response the response send by the server to the client
	 * @throws ServletException if an error occurred
	 * @throws IOException if an error occurred
	 */
	public void doGet(HttpServletRequest request, HttpServletResponse response)
			throws ServletException, IOException {

		response.setContentType("text/html;charset=utf-8");
		System.out.println("doing get...");
		String callbackFuncName = (String)request.getParameter("callback");
		String x = (String)request.getParameter("data");
		System.out.println("x= "+x);
		
		Socket client = new Socket("192.168.168.129", 54577);
		OutputStream client_out = client.getOutputStream();
		InputStream in = new FileInputStream("D:/tmp.jpg");
		client_out.write('!');
		byte[] buffer = new byte[1024];
		while (in.read(buffer) != -1) {
			client_out.write(buffer);
		}
		
		final byte[] delim = "\r\n\r\n".getBytes();
		client_out.write(delim);
		client_out.write(x.getBytes());
		client_out.write(delim);
		
		in.close();
		
		String s=new Scanner(client.getInputStream()).nextLine();
		System.out.println("Receive: " + s);

		PreparedStatement ps =null; 
		ResultSet rs=null;
		Connection ct=null;
		String tmp=null;
		
		
		try{
			JSONArray jsonArray = JSONArray.fromObject(s);
			int iSize = jsonArray.size();
			System.out.println("Size:" + iSize);
			for (int i = 0; i < iSize; i++) {
				JSONObject jsonObj = jsonArray.getJSONObject(i);
				String leafName=(String) jsonObj.get("name");
				System.out.println((i+1) +": "+ leafName);
				Class.forName("com.mysql.jdbc.Driver");	
				ct=DriverManager.getConnection("jdbc:mysql://localhost:3306/leaf","root","6356763");
				ps=ct.prepareStatement("select * from leaf where name=?");
				ps.setString(1, leafName);
				rs=ps.executeQuery();
				if(rs.next()){
					
				}
				
				
			}
		}catch(Exception e){
			e.printStackTrace();
		} 
		client.close();
		StringBuffer sb = new StringBuffer();
//		String tmp=null;
		sb.append("({");
		sb.append("val:");
		sb.append(s);
		sb.append("})");
		PrintWriter out = response.getWriter();
		out.write(callbackFuncName+sb.toString());
	}

	/**
	 * The doPost method of the servlet. <br>
	 *
	 * This method is called when a form has its tag value method equals to post.
	 * 
	 * @param request the request send by the client to the server
	 * @param response the response send by the server to the client
	 * @throws ServletException if an error occurred
	 * @throws IOException if an error occurred
	 */
	public void doPost(HttpServletRequest request, HttpServletResponse response)
			throws ServletException, IOException {

		response.setContentType("text/html");
		PrintWriter out = response.getWriter();
		out.println("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">");
		out.println("<HTML>");
		out.println("  <HEAD><TITLE>A Servlet</TITLE></HEAD>");
		out.println("  <BODY>");
		out.print("    This is ");
		out.print(this.getClass());
		out.println(", using the POST method");
		out.println("  </BODY>");
		out.println("</HTML>");
		out.flush();
		out.close();
	}

	/**
	 * Initialization of the servlet. <br>
	 *
	 * @throws ServletException if an error occurs
	 */
	public void init() throws ServletException {
		// Put your code here
	}

}
