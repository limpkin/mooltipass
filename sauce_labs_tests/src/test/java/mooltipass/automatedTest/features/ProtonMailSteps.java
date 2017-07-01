package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.ProtonMail;

public class ProtonMailSteps {
	ProtonMail proton= new ProtonMail(WebDriverFactory.get());

	@When("I login ProtonMail with '(.*)'")
	public void login(String username){
		proton.goToLogin();
		proton.enterEmail(username);
		String password =System.getenv().get("PROTONPASS");
		proton.enterPassword(password);
		proton.submit();
		
	}
	@When("I go to ProtonMail login page")
	public void pressLogin(){
		Assert.assertTrue("Expected to be at login page", proton.checkAtLoginPage());
		
	}
	@Then("I should be logged in ProtonMail")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",proton.checkLogin());
	}
	
	@When("I logout ProtonMail")
	public void pressLogout(){
		proton.goTodDashboard();
		proton.logout();
	}
}
