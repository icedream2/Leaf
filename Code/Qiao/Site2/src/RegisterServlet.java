import java.io.IOException;
import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


public class RegisterServlet extends HttpServlet {

	public RegisterServlet() {
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
		//System.out.println("do get");
		response.setContentType("text/html");
		String callbackFuncName = (String)request.getParameter("callback");
		String name = (String)request.getParameter("register_name");
		String password=(String )request.getParameter("register_password1");
		//System.out.println(name);
		//System.out.println(password);
		PreparedStatement ps =null; 
		ResultSet rs=null;
		Connection ct=null;
		String tmp=null;
		try {
			Class.forName("com.mysql.jdbc.Driver");	
			ct=DriverManager.getConnection("jdbc:mysql://localhost:3306/leafuser","root","6356763");
			ps=ct.prepareStatement("select * from user where id=?");
			ps.setString(1, name);
			rs=ps.executeQuery();
			if(rs.next()){
				tmp="fail";
			}else{
				PreparedStatement ps1=ct.prepareStatement("insert into user value(?,?)");
				System.out.println("insert user "+name);
				ps1.setString(1, name);
				ps1.setString(2, password);
				ps1.executeUpdate();
				tmp="success";
			}
					
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		}
		
		StringBuffer sb = new StringBuffer();
		sb.append("({");
		sb.append("val:");
		sb.append("'");
		sb.append(tmp);
		sb.append("'");
		sb.append("})");
		System.out.println(callbackFuncName);
		System.out.println(sb.toString());
		PrintWriter out = response.getWriter();
		out.write(callbackFuncName + sb.toString());
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

		response.setContentType("text/html;charset=utf-8");
		String name = request.getParameter("login_name");
		String password = request.getParameter("login_password");
		System.out.println(name);
		System.out.println(password);
		PrintWriter pw = response.getWriter();
		System.out.println(pw);
		pw.close();
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
